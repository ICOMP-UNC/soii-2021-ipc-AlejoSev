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

![trama](./img/trama.png)

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

Para poder empezar a recibir clientes, lo primero que tenemos que hacer, es agregar el `listen_sock` a nuestra lista de intereses de `epoll_fd`. De esta forma, cada vez que nos llegue un evento a nuestra instancia de epoll, podemos verificar si el file descriptor que lo generó es el `listen_sock`. En caso de ser así, significa que un cliente realizó un `connect()` hacia el socket de nuestro servidor. Una vez que detectamos ese intento de conexión, lo aceptamos, lo agregamos a nuestra lista de interés (logrando e esta forma que cuando quiera enviar un mensaje el cliente, se genere un evento) y le mandamos un primer mensaje. Este primer mensaje es un **Checksum_Request**, mediante el cual podemos confirmar, en caso de que nos devuelva un **Checksum_Acknowledge**, que hay una buena comunicación establecida con ese cliente y así agregarlo a la lista `connected_clients`.

A partir de este punto, el cliente cada vez que recibe un mensaje del servidor, en caso de que el campo de Hash coincida con el campo Data luego de ser pasado por md5, devuelve un **Message_Acknowledge** para confirmar la correcta recepción del mensaje del servidor.

### Command Line Interface - Servidor

La comunicación con la CLI funciona exactamente de la misma manera que con el resto de los clientes, solo que chequea la correcta recepción del primer mensaje (Checksum_Request) y a partir de ahí no recibe mas mensajes de parte del servidor, sino que sólo le envía.

### Productores - Servidor

Para esta comuncación utilicé, como ya mencioné anteriormente, una **Message Queue** de **System V**. La Message Queue es creada por el server, utilizando una **Key** generada a partir del path al archivo *server.c*. De esta forma, los productores pueden unirse a la misma Message Queue utilizando también la Key generada con el mismo path. Una vez que los productores se unen a la misma, generan los mensajes y los envían a la misma, aplicando cada uno un tipo de mensaje distinto. Para el **Productor1** utilicé un **mtype = 2**, para el **Productor2** un **mtype = 3** y para el **Productor3** un **mtype = 4**, consiguiendo así poder distinguir cuál fue el productor que envió cada mensaje dentro de la Message Queue.

## Correcta utilización del programa

Primero que nada, tenemos que levantar el servidor

`./server <port>`

El argumento `port` puede tomar cualquiera de los puertos disponibles en el host donde está corriendo el servidor.

Una vez levantado podemos conectar cualquiera de los demás, sin importar el orden

`./cli <hostname> <port>`

`./productor1`

`./productor2`

`./productor3`

`./client <hostname> <port> <address>`

Siendo `hostname` el nombre del host donde se está ejecutando el código, `port` el mismo puerto que se le asignó al servidor y `address` la dirección que le queremos asignar al cliente.

También podemos levantar muchos clientes a la vez utilizando el **Batch File**

`./run_clients.sh <hostname> <port> <number>`

Donde hostname y port funcionan de la misma manera que en `./client`, y `number` es la cantidad de clientes que queremos levantar. Tener en cuenta que la `address` de cada cliente se asignará de forma automática, empezando por la dirección **0** hasta la dirección **number-1**.

Una vez levantado todo, podemos empezar a subscribir clientes a productores. Para eso usaremos la CLI:

`add <client_address> <productor>`

`delete <client_address> <productor>`

`log <client_address>`

Siendo `client_address` la dirección de un cliente levantado previamente con `./client`, y `productor` cualquiera de los 3 valores posibles (`productor1`, `productor2` y `productor3`).

Cabe aclarar que se puede subscribir un mismo cliente a más de un productor, pero esto debe hacerse con varios `add`. Lo mismo para `delete`.

Una vez finalizado el programa, para evitar errores es necesario cerrarlo de la siguiente manera:

Primero se cierran los productores, la CLI y los clientes (sin importar el orden). Y luego se cierra el servidor.

También hay a disposición un **Batch File** para cerrar todo, `./stop.sh`.

Una vez finalizado el programa, se pueden ver **conexiones**, **desconexiones**, **subscripciones**, **desubscripciones**, **envíos de mensajes**, **inicio y finalización del servidor** de forma cronológica en el archivo **log.txt**.