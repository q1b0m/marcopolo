package avion;
import java.rmi.RemoteException;
import java.rmi.server.UnicastRemoteObject;
import java.util.concurrent.ArrayBlockingQueue;


// La clase AvionImpl expone vía RMI el método pistaConcedida() que será invocado
// desde la torre de control cuando se conceda pista al avión, y recibirá como
// parámetro la pista concedida.

// Además proporciona el método getPista() para que pueda ser llamado desde
// el simulador de los vuelos, para así obtener la pista concedida.

// Estos dos métodos se ejecutan en diferentes hilos (pistaConcedida() se
// ejecuta en el hilo que atiende peticiones RMI, mientras que getPista() se
// ejecuta en el hilo del simulador de vuelos). Se comunican entre sí mediante
// una cola bloqueante. getPista() intenta leer de esa cola, y si está vacía
// se bloquea. pistaConcedida() pone en esa cola la pista recibida, 
// desbloqueando así al hilo que estaba en getPista()
public class AvionImpl extends UnicastRemoteObject implements AvionInterface {
    private ArrayBlockingQueue<Integer> cola;  // Cola bloqueante para comunicar los hilos

    public AvionImpl() throws RemoteException {
        super();
        cola = new ArrayBlockingQueue<Integer>(1);  // Basta que tenga tamaño 1
    }

    @Override
    public void pistaConcedida(int pista) throws RemoteException {
        // Este método es invocado vía RMI desde la torre de control
        // Recibe la lista concedida y se limita a guardarla en la cola interna

        // A RELLENAR
        this.cola.add(pista);
    }

    public int getPista() throws InterruptedException {
        // Este método es invocado desde el simulador
        // Espera a que aparezca la pista concedida en la cola interna
        // y retorna su valor

        // A RELLENAR
        return this.cola.take();
    }
}
