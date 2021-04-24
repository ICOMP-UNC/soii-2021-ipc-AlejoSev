Trabajo Práctico 1 - Sistemas Operativos II
### Ingeniería en Computación - FCEFyN - UNC
# Inter-Process Communication

## Elección de IPC
### Clientes - Servidor (Delivery Manager)

Para la comunicación entre los Clientes y el Servidor utilicé (como pedía la consigna) Sockets. La forma más óptima de implementarlo que encontré fue mediante la API [epoll](https://man7.org/linux/man-pages/man7/epoll.7.html). Las otras opciones que tenía disponible eran [select](https://man7.org/linux/man-pages/man2/select.2.html) o [poll](https://man7.org/linux/man-pages/man2/poll.2.html), pero estas dos quedaron descartadas ya que no cumplían con el requirimiento de 5000 clientes conectados a la vez. En cambio epoll permite manejar 5000 e incluso más, el límite se encuentra en las configuraciones de nuestro sistema operativo. Además de que epoll tiene un desempeño muchisimo mejor en cuanto a tiempos a medida que se aumenta el número de file descriptors que se está controlando con respecto a los otros dos.

### Command Line Interface - Servidor

En este caso utilicé Sockets también, aprovechando que el uso de epoll hace muy fácil el manejo de file descriptors. De esta forma pude tratar a la CLI como un cliente mas que se conecta, solo que el servidor al detectar que es la CLI quien se está comunicando, le da un trato distinto que al resto de clientes manejados por epoll.

### Productores - Servidor

Para la comunicación entre Productores y Servidor elegí Message Queue de [System V](https://man7.org/linux/man-pages/man7/svipc.7.html). Opté por ésta ya que los mensajes pueden llevar un ID, el cual me serviría para identificar de cuál productor es el mensaje obtenido (todos los productores escriben en la misma Message Queue).

## Protocolo y Trama

En el envío de mensajes entre Clientes - Servidor y CLI - Servidor utilicé la siguiente trama: 

![trama](/img/trama.png)

- **Client Type:** H para los clientes (Hosts) y C para la CLI.
- **Client Address:** Dirección del cliente.
- **Client Port:** Puerto del cliente.
- **Data:** Es el mensaje propiamente dicho.
- **Hash:** Es el resultado de pasar Data por el agoritmo de encriptado MD5.

La implementación del Hash dentro del protocolo es para que los clientes al recibir un mensaje, comparen el resultado de pasar ellos mismos Data por el algoritmo md5  con el campo Hash, y en caso de ser iguales, asegurarse de que el campo Data recibido no fue alterado durante la transmisión.

## Implementacion de los IPC
### Clientes - Servidor

La implementacion de Sockets como IPC fue el siguiente:

En el servidor se crea un socket llamado `listen_sock`, el cual se encarga de recibir los intentos de conexión de los clientes. 

Luego se crea una instancia de epoll, el cual devuelve un file descriptor, en este caso llamado `epoll_fd`, en esta instancia de epoll tenemos una lista de interés, en la cual podemos cargar los file descriptors que nos interesa escuchar. Estos file descriptors van a generar eventos, los cuales trataremos dependiendo del tipo de evento.

Para poder empezar a recibir clientes, lo primero que tenemos que hacer, es agregar el `listen_sock` a nuestra lista de intereses de `epoll_fd`. De esta forma, cada vez que nos llegue un evento a nuestra instancia de epoll, podemos verificar si el file descriptor que lo generó es el `listen_sock`. En caso de ser así, significa que un cliente realizó un `connect()` hacia el socket de nuestro server. Una vez que detectamos ese intento de conexión, lo aceptamos, los guardamos en una lista de clientes conectados, lo agregamos a nuestra lista de interés (logrando e esta forma que cuando quiera enviar un mensaje el cliente, se genere un evento) y le mandamos un primer mensaje. Este primer mensaje es un **Checksum_Request**, mediante el cual podemos confirmar, en caso de que nos devuelva un **Checksum_Acknowledge**, que hay una buena comunicación establecida con ese cliente.