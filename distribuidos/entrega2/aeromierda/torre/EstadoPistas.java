package torre;

// Primitivas de sincronización
import java.util.concurrent.Semaphore;

// Clase que mantiene el estado de las pistas, y permite modificarlo a
// través de sus métodos. Ya que estos métodos pueden ser llamados
// desde diferentes hilos, se sincronizan entre sí mediante
// bloques syncrhonized (exclusión mutua) y mediante un semáforo (para
// señalar cuándo hay pista libre)

class EstadoPistas {
    private int estado[];   // Array de enteros: 0 pista libre, 1 pista ocupada
    private int size;       // Numero de pistas
    private Semaphore num_huecos;  // Semáforo para señalar existencia de huecos

    public EstadoPistas(int size) {
        // El constructor inicializa el array y el semáforo
        num_huecos = new Semaphore(size);
        this.size = size;
        estado = new int[size];
        for (int i=0; i<size; i++) {
            estado[i] = 0;
        }
    }

    // Función de utilidad para mostrar el estado de las pistas
    synchronized void print_pistas(String msg) {
        System.out.print(msg + ": [");
        for (int p : estado) {
            System.out.print(p);
        }
        System.out.println("]");
    }

    // Función que espera a que haya pista libre y retorna su número
    int buscar_pista() throws InterruptedException {
        int encontrada = -1;
        print_pistas("EstadoPistas.buscar_pista (antes)");

        // Espera en el semáforo a que haya pista libre
        num_huecos.acquire();
        synchronized(this) {                                // Exclusión mutua
            // Accede al array buscando un sitio libre
            for (int i=0; i < size; i++) {
                if (estado[i] == 0) {
                    estado[i] = 1;
                    encontrada = i;
                    break;
                }
            }
        }                                                  // Fin exclusión

        print_pistas(String.format("EstadoPistas. Encontrada pista %d", encontrada));
        return encontrada;
    }

    // Pone un 0 en la pista y señala en el semáforo
    synchronized void liberar_pista (int pista){  // La función entera opera en exclusión mutua
        if (estado[pista] == 0) {
            System.out.println("WARNING: Intento de liberar pista "+pista+" que estaba libre");
            return;
        }
        estado[pista] = 0;
        num_huecos.release();
        print_pistas(String.format("EstadoPistas. Liberada pista %d", pista));
    }
}
