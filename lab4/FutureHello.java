/* Disciplina: Programacao Concorrente */
/* Prof.: Silvana Rossetto */
/* Laboratório: 11 */
/* Codigo: Exemplo de uso de futures */
/* Nicolly Zorzam Moura - DRE: 121037550 */
/* -------------------------------------------------------------------*/

import java.util.concurrent.Callable;
import java.util.concurrent.ExecutionException;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.Future;

import java.util.ArrayList;
import java.util.List;


//classe runnable
class MyCallable implements Callable<Long> {
  //construtor
  MyCallable() {
  }
 
  //método para execução
  public Long call() throws Exception {
    long s = 0;
    for (long i=1; i<=100; i++) {
      s++;
    }
    return s;
  }
}

// classe que verifica se um numero eh primo
class VerificaPrimo implements Callable<Integer> {
  private final long n;

  public VerificaPrimo(long n) {
    this.n = n;
  }

  public Integer call() throws Exception{
    if (ehPrimo(n)) return 1;
    return 0;
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
}


//classe do método main
public class FutureHello  {
  private static final int N = 1000000;
  private static final int NTHREADS = 10;

  public static void main(String[] args) {
    //cria um pool de threads (NTHREADS)
    ExecutorService executor = Executors.newFixedThreadPool(NTHREADS);
    //cria uma lista para armazenar referencias de chamadas assincronas
    List<Future<Long>> list = new ArrayList<Future<Long>>();
    List<Future<Integer>> list_primo = new ArrayList<Future<Integer>>();

    for (int i = 0; i < N; i++) {
      Callable<Long> worker = new MyCallable();
      Future<Long> submit = executor.submit(worker);
      list.add(submit);
      
      Callable<Integer> worker_primo = new VerificaPrimo(i);
      Future<Integer> submit_primo = executor.submit(worker_primo);
      list_primo.add(submit_primo);
    }

    System.out.println(list.size());
    //pode fazer outras tarefas...

    //recupera os resultados e faz o somatório final
    long sum = 0;
    int totalPrimos = 0;
    
    for (Future<Long> future : list) {
      try {
        sum += future.get(); //bloqueia se a computação nao tiver terminado
      } catch (InterruptedException e) {
        e.printStackTrace();
      } catch (ExecutionException e) {
        e.printStackTrace();
      }
    }

    for (Future<Integer> future : list_primo) {
      try {
        totalPrimos += future.get(); //bloqueia se a computação nao tiver terminado
      } catch (InterruptedException e) {
        e.printStackTrace();
      } catch (ExecutionException e) {
        e.printStackTrace();
      }
    }

    System.out.println(sum);
    System.out.println("Total de primos: " + totalPrimos);

    executor.shutdown();
  }
}
