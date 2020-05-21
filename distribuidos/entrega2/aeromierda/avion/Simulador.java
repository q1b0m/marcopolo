package avion;

// Imports necesarios para RabbitMQ
import com.rabbitmq.client.Channel;
import com.rabbitmq.client.Connection;
import com.rabbitmq.client.ConnectionFactory;

// Imports necesarios para RMI
import java.io.IOException;
import java.rmi.Naming;

public class Simulador {
  private final static String NOMBRE_COLA = "solicitar_pista";
  
  public static void main(String[] argv) throws Exception {
    int id = 0;          // Identificador del avión
    int num_vuelos = 0;  // Número de vuelos a efectuar

    // =================================================
    // Obtener argumentos de línea de comandos, verificando que
    // hay el número adecuado de ellos y que son números enteros
    // A RELLENAR

    if (argv.length != 2) {
      System.out.println("Usage: Simulador <id> <num_vuelos>");
      System.exit(1);
    }

    try {
      id = Integer.parseInt(argv[0]);
      num_vuelos = Integer.parseInt(argv[1]);

    } catch (NumberFormatException e) {
      System.out.println("Argumentos de tipo entero");
      System.exit(1);
    }

    // =================================================
    // Instanciar SecurityManager necesario para RMI
    if (System.getSecurityManager() == null) {
      System.setSecurityManager(new SecurityManager());
    }

    // =================================================
    // Parte principal, toda dentro de un try para capturar cualquier excepción
    try {
      // Arrancar el servidor RMI AvionImpl y registrarlo ante rmiregistry
      // con el nombre Avion_id según su id
      // A RELLENAR
      String mibase = System.getenv("HOME");
      mibase = "file:" + mibase + "/RMI/servidor/clases/"; // CAMBIAR
      System.setProperty("java.rmi.server.codebase", mibase);
      
      AvionImpl avion = new AvionImpl();
      String objBind = "Avion_" + Integer.toString(id)
      
      Naming.rebind(objBindb, avion);
      System.out.println(objBindb + "registrado");


      // Conectar con Rabbit para poder enviar peticiones a la cola
      // A RELLENAR

      // Realizar la simulación
      simular_vuelos(num_vuelos, channel, id, avion);

      // Terminar
      System.out.println("Avion " + id + " finalizó y se va al hangar.");
      channel.close();
      connection.close();
      System.exit(0);
    } 
    catch (Exception e) {
      // Cualquier excepción simplemente se imprime
      System.out.println("Error en Avion_" + id + ": " + e.getMessage());
      e.printStackTrace();
    }
  }

  static double randRange(double min, double max) {
    // Función de utilidad para generar un double entre min y max
    return min + Math.random() * (max - min);
  }

  // =========================================================================
  // La función que simula las diferentes operaciones del avion
  // Requiere como parámetros:
  //
  //  - El número de vuelos a realizar
  //  - El canal de comunicación con RabbitMQ para pedir pista
  //  - El id del avión, pues lo usa como parte del mensaje enviado a RabbitMQ
  //  - El objeto AvionImpl, pues lo usa para obtener (esperar) la pista libre
  // 
  // Repite num_vuelos veces el bucle de simulación  y retorna
  static void simular_vuelos(int num_vuelos, Channel channel, int id, AvionImpl avion) 
              throws IOException, InterruptedException 
  {
    String msg;       // Para crear los mensajes a enviar por Rabbit
    int pista;        // Para obtener la pista concedida
    double t;          // Tiempos de uso de pista, vuelo, etc.

    for (int vuelo=0; vuelo<num_vuelos; vuelo++) {
  
      // Vamos a despegar
      System.out.println(String.format("Avion %d solicita pista para despegue (vuelo %d)", id, vuelo));

      // Crear mensaje apropiado y ponerlo en la cola RabbitMQ
      // A RELLENAR

      // Esperar por la pista asignada
      pista = avion.getPista();

      // Simular que se tarda un tiempo aleatorio en despegar
      t = randRange(1, 3);
      System.out.println(String.format("Avion %d usa pista %d para despegar durante %g segundos (vuelo %d)", 
                                        id, pista, t, vuelo));
      Thread.sleep(Math.round(t*1000));
      System.out.println(String.format("Avion %d libera pista %d tras despegar (vuelo %d)",  id, pista, vuelo));

      // Notificar liberación de la pista
      // Crear mensaje apropiado y ponerlo en la cola RabbitMQ
      // A RELLENAR


      // Simula volar
      t = randRange(3, 8);
      System.out.println(String.format("Avion %d esta volando durante %fs (vuelo %d)", id, t, vuelo));
      Thread.sleep(Math.round(t*1000)); 
    
      // Aterrizaje
      System.out.println(String.format("Avion %d solicita pista para aterrizaje (vuelo %d)", id, vuelo));
      // Crear mensaje apropiado y ponerlo en la cola RabbitMQ
      // A RELLENAR

      // Esperar por la pista asignada
      pista = avion.getPista();
    
      // Simular que se usa un tiempo aleatorio
      t = randRange(1, 3);
      System.out.println(String.format("Avion %d usa pista %d para aterrizar durante %g segundos (vuelo %d)", 
                                        id, pista, t, vuelo));
      Thread.sleep(Math.round(t*1000));
      System.out.println(String.format("Avion %d libera pista %d tras despegar (vuelo %d)",  id, pista, vuelo));

      // Notificar liberación de la pista
      // Crear mensaje apropiado y ponerlo en la cola RabbitMQ
      // A RELLENAR

    } // Volver al bucle a simular otro vuelo
    // Terminados todos los vuelos, la función retorna
  }
}
