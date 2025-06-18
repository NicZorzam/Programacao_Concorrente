/* Disciplina: Programacao Concorrente */
/* Prof.: Silvana Rossetto */
/* Laboratório: 11 */
/* Codigo: Criando um pool de threads em Java */
/* Nicolly Zorzam Moura - DRE: 121037550 */

import java.util.LinkedList;

//-------------------------------------------------------------------------------
// Classe que implementa um pool de um número fixo de threads que consomem as tarefas da fila
class FilaTarefas {
    private final int nThreads;
    private final MyPoolThreads[] threads;
    private final LinkedList<Runnable> queue; // fila de tarefas
    private boolean shutdown;
    // Construtor da classe
    public FilaTarefas(int nThreads) {
        this.shutdown = false;
        this.nThreads = nThreads;
        queue = new LinkedList<Runnable>();
        threads = new MyPoolThreads[nThreads];
        for (int i=0; i<nThreads; i++) {
            threads[i] = new MyPoolThreads();
            threads[i].start(); // inicia a execucao da thread
        } 
    }
    // método que adiciona tarefas a fila
    public void execute(Runnable r) {
        synchronized(queue) {
            if (this.shutdown) return; // não aceita mais tarefas se this.shutdown = true
            queue.addLast(r); // adiciona a tarefa ao final da fila
            queue.notify(); // acorda uma thread que esteja esperando por tarefas
        }
    }
    // método que finaliza o pool
    public void shutdown() {
        synchronized(queue) {
            this.shutdown=true;
            queue.notifyAll(); // acorda todas as threads
        }
        for (int i=0; i<nThreads; i++) { // espera o término de cada thread
          try { threads[i].join(); } catch (InterruptedException e) { return; }
        }
    }
    // classe interna de cada thread que consome e executa as tarefas da fila
    private class MyPoolThreads extends Thread {
       public void run() {
         Runnable r;
         while (true) {
           synchronized(queue) {
             while (queue.isEmpty() && (!shutdown)) {
               try { queue.wait(); } // thread aguarda ate que haja alguma tarefa ou shutdown = true
               catch (InterruptedException ignored){}
             }
             if (queue.isEmpty()) return; // shutdown = true e fila vazia
             r = (Runnable) queue.removeFirst(); // retira a próxima tarefa da fila
           }
           try { r.run(); }
           catch (RuntimeException e) {}
         } 
       } 
    } 
}
//-------------------------------------------------------------------------------

//--PASSO 1: cria uma classe que implementa a interface Runnable 
class Hello implements Runnable {
   String msg;
   public Hello(String m) { msg = m; }

   //--metodo executado pela thread
   public void run() {
      System.out.println(msg);
   }
}

class Primo implements Runnable {
   private final long n;

   public Primo(long n) {
        this.n = n;
    }

    //funcao para determinar se um numero eh primo
    private boolean ehPrimo(long n) {
        long i;
        if (n <= 1) return false;
        if (n == 2) return true;
        if (n % 2 == 0) return false;
        for(i = 3; i < Math.sqrt(n) + 1; i+=2) {
            if (n % i == 0) return false;
        }
        return true;
    }
    
   public void run() {
    if (ehPrimo(n)) {
            System.out.println(n + " eh primo");
        } else {
            System.out.println(n + " nao eh primo");
        }
   }
}

//Classe da aplicação (método main)
class MyPool {
    private static final int NTHREADS = 10;

    public static void main (String[] args) {
      //--PASSO 2: cria o pool de threads
      FilaTarefas pool = new FilaTarefas(NTHREADS); 
      
      //--PASSO 3: dispara a execução dos objetos runnable usando o pool de threads
      for (int i = 0; i < 25; i++) {
        final String m = "Hello da tarefa " + i;
        Runnable hello = new Hello(m);
        pool.execute(hello);
        Runnable primo = new Primo(i);
        pool.execute(primo);
      }

      //--PASSO 4: esperar pelo termino das threads
      pool.shutdown();
      System.out.println("Terminou");
   }
}
