package torre;

// Imports necesarios para usar RabbitMQ
import com.rabbitmq.client.Channel;
import com.rabbitmq.client.Connection;
import com.rabbitmq.client.ConnectionFactory;
import com.rabbitmq.client.Consumer;
import com.rabbitmq.client.DefaultConsumer;
import com.rabbitmq.client.Envelope;
import com.rabbitmq.client.AMQP;

// Imports necesarios para usar RMI
import java.io.IOException;
import java.rmi.Naming;
import java.rmi.RemoteException;

// Imports necesarios para invocar via RMI métodos del avion
//import avion.AvionInterface;

// Cola bloqueante para comunicar al controlador con los Gestores de entrada y salida
import java.util.concurrent.ArrayBlockingQueue;

// ===================================================================
// Las tres clases siguientes son hilos que se ejecutarán de forma concurrente
//
// - Controlador es el hilo que espera mensajes de RabbitMQ y según el mensaje
//   recibido, lo despacha a uno de los otros dos hilos
// - GestorSolicitudes es el hilo que gestiona las peticiones de pista
// - GestorLiberaciones es el hilo que gestiona las liberaciones de pista
//
// El Controlador se comunica con los otros dos mediante colas bloqueantes
// que comparten.
//
// Los Gestores comparten una instancia de EstadoPistas que usan para alterar
// los estados de cada pista.


// Clase Controlador. Recibe mensajes por RabbitMQ, los analiza y mete en colas
// bloqueantes las solicitudes para que sean atendidas en los hilos Gestores
class Controlador extends Thread {
    private final static String NOMBRE_COLA_RABBIT = "solicitar_pista";
    private ArrayBlockingQueue<Integer> cola_solicitar;
    private ArrayBlockingQueue<Integer> cola_liberar;

    // El constructor recibe las colas bloqueantes que le permiten comunicarse
    // con los otros hilos
    public Controlador(ArrayBlockingQueue<Integer> cola_solicitar,
                       ArrayBlockingQueue<Integer> cola_liberar){
        this.cola_solicitar = cola_solicitar;
        this.cola_liberar = cola_liberar;
    }

    // La función run es la que se ejecuta al poner en marcha el hilo
    public void run() {
        // Conectar con rabbitMQ
        ConnectionFactory factory = new ConnectionFactory();
        factory.setHost("localhost");
        try {
            Connection connection = factory.newConnection();
            Channel channel = connection.createChannel();
            channel.queueDeclare(NOMBRE_COLA_RABBIT, false, false, false, null);

            // Espera por peticiones en la cola rabbitMQ
            Consumer consumer = new DefaultConsumer(channel) {
                @Override
                public void handleDelivery(String consumerTag, Envelope envelope, AMQP.BasicProperties properties, byte[] body) throws IOException {
                    // ************************************************************
                    // Recepción y manejo del mensaje que llega por RabbitMQ
                    // ************************************************************

                    // Convertir en cadena el mensaje recibido
                    String solicitud = new String(body, "UTF-8");
                    System.out.println("Controlador: Recibido mensaje = " + solicitud);
                    // Ejemplos de mensajes recibidos 
                    //  - "SP 2" (solicitud de pista del avión 2)
                    //  - "LP 3" (liberación de la pista 3)

                    // Procesar el mensaje separando sus partes por los espacios. 
                    // Si la primera parte es "SP", extraer (de la segunda parte) el número de avión
                    // y enviárselo al GestorPeticiones
                    // Si la primera parte es "LP" extraer (de la segunda parte) el número de pista
                    // y enviárselo al GestorLiberaciones
                    // En otro caso ignorar el mensaje (se puede emitir un WARNING por la salida estándar)

                    // A RELLENAR
                    String action = "";

        			action += solicitud.charAt(0);
        			action += solicitud.charAt(1);

        			int dato = Integer.parseInt(solicitud.substring(3));

        			if (action.equals("SP")) {
        				this.cola_solicitar.add(Integer.parseInt(dato));
        			}

        			if (action.equals("LP")) {
        				this.cola_liberar.add(Integer.parseInt(dato));
        			}

                }
            };
            System.out.println("Torre de control. Esperando solicitudes de pista");
            channel.basicConsume(NOMBRE_COLA_RABBIT, true, consumer);
            
        } catch (Exception e) {  // No manejamos excepciones, simplemente abortamos
            e.printStackTrace();
            System.exit(1);
        }
    }
}

// Clase GestorSolicitudes espera en una cola bloqueante a que el Controlador
// le envíe una petición de pista de un avión.
// Solicita al objeto EstadoPistas que le dé una y se la notifica al avión 
// adecuado haciendo una invocación RMI
class GestorSolicitudes extends Thread {
    private ArrayBlockingQueue<Integer> cola;  // Cola bloqueante en que espera peticiones del Controlador
    private EstadoPistas estado_pistas;        // Objeto que mantiene el estado de las pistas

    // El constructor recibe la cola y el estado de las pistas
    public GestorSolicitudes(ArrayBlockingQueue<Integer> cola, EstadoPistas estado_pistas) {
        this.cola = cola;
        this.estado_pistas = estado_pistas;
    }

    // El método run es el que se ejecuta al arrancar el hilo
    public void run() {
        try {
            while (true) {  // Bucle infinito
                // Esperar solicitud del Controlador en la cola bloqueante
                // A RELLENAR
                int id_avion = this.cola.take();

                // Obtener pista libre de EstadoPistas. Esta llamada es bloqueante
                // (no se prosigue hasta que haya pista libre)
                // A RELLENAR
                int pista = this.estado_pistas.buscar_pista();

                // Notificarlo al avión a través de RMI al avión adecuado
                // Primero se obtiene la instancia remota (del Avion_id) que corresponda
                // y luego se invoca su método pistaConcedida()
                try {
                    
                    // A RELLENAR
                	String objRef = "Avion_" + Integer.toString(id_avion);

                    AvionInterface obj = 
                    		(AvionInterface) Naming.lookup(objRef);
        
                	obj.pistaConcedida(pista);

                } catch (RemoteException e) {
                	System.out.println("GestorSolicitudes: Error en la " +
                			"invocación remota: "+ e.getMessage());
                    e.printStackTrace();
                }
            }

        } catch (Exception e) {
            e.printStackTrace();
            System.exit(2);
        }
    }
}

// Clase GestorSolicitudes espera en una cola bloqueante a que el Controlador
// le envíe una petición de liberación de pista.
// Solicita al objeto EstadoPistas que la libere. No necesita notificar nada
class GestorLiberaciones extends Thread {
    private ArrayBlockingQueue<Integer> cola;  // Cola bloqueante para recibir comandos del Controlador
    private EstadoPistas estado_pistas;        // Objeto que mantiene el estado de las pistas

    // El constructor recibe la cola y el estado de las pistas
    public GestorLiberaciones(ArrayBlockingQueue<Integer> cola, EstadoPistas estado_pistas) {
        this.cola = cola;
        this.estado_pistas = estado_pistas;
    }

    // El método run es el que se ejecuta al arrancar el hilo
    public void run() {
        try {
            while (true) {  // Bucle infinito de espera
                // Esperar solicitud de pista a liberar
                Integer solicitud = cola.take();

                // Liberarla a través del objeto estado_pistas
                estado_pistas.liberar_pista(solicitud);
            }
        } catch (Exception e) {
            e.printStackTrace();
            System.exit(2);
        }
    }
}


// Clase principal que instancia los hilos anteriores y los arranca
public class Torre {   
    public static void main(String[] argv) throws Exception {
        int max_aviones;    // Estas variables se leen de línea de comandos
        int max_pistas;

        // Lectura de la línea de comandos
        // Se debe comprobar que se reciben dos parámetros (max_aviones y max_pistas)
        // emitiendo un error si falta alguno, o si no son de tipo numérico
        // A RELLENAR	
        if (argv.length != 2) {
        	System.out.println("Usage: torre <max_aviones> <max_pistas>");
        	System.exit(1);
        }
		
		try {
            max_aviones = Integer.parseInt(argv[0]);
            max_pistas = Integer.parseInt(argv[1]);

        } catch (NumberFormatException e) {
            System.out.println("Argumentos de tipo entero");
            System.exit(1);
        }
        
        // Primero se crean las colas internas de sincronización entre hilos
        ArrayBlockingQueue<Integer> cola_solicitar =  new ArrayBlockingQueue<Integer>(max_aviones);
        ArrayBlockingQueue<Integer> cola_liberar = new ArrayBlockingQueue<Integer>(max_aviones);

        // Y el objeto que mantiene y modifica el estado de las pistas
        EstadoPistas estado_pistas = new EstadoPistas(max_pistas);     
        
        // Manager de seguridad para RMI
        if (System.getSecurityManager() == null) {
            System.setSecurityManager(new SecurityManager());
        }

        // Ahora se crean los hilos, pasándole a cada uno los parámetros apropiados
        Controlador controlador = new Controlador(cola_solicitar, cola_liberar);
        GestorSolicitudes gestorSolicitudes = new GestorSolicitudes(cola_solicitar, estado_pistas);
        GestorLiberaciones gestorLiberaciones = new GestorLiberaciones(cola_liberar, estado_pistas);

        // Arrancamos los hilos
        controlador.start();  
        gestorSolicitudes.start();
        gestorLiberaciones.start();

        // Esperamos a que finalice el controlador (nunca finalizará, hay que parar con Ctrl+C)
        controlador.join();
   
    }
}