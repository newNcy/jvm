

public class Test{
	public static void main(String[] args) throws Exception {
		final int count = 10;
		Thread[] threads = new Thread[count];
		for (int i = 0; i < count; i++) {
			threads[i] = new RandomThread("RandomThread:" + i);
		}
		for(Thread thread : threads) {
			thread.start();
		}
		System.out.println("main exit");
		throw new Exception("just throw");
	}
}

class RandomThread extends Thread {

	public RandomThread(String name) {
		super(name);
	}

	@Override
	public void run() {
		System.out.println(getName());
	}
}
