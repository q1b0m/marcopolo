package avion;
import java.rmi.Remote;
import java.rmi.RemoteException;

// Interfaz remoto del Avion que ha de hacer público el método pistaConcedida
// para que la torre de control pueda invocarlo vía RMI
public interface AvionInterface extends Remote {
    // A RELLENAR
    public void pistaConcedida(int pista) throws java.rmi.RemoteException {
}
