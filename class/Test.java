

class A
{
	public int c = 0;
}

class Test extends A
{
	int a = 0;
	public int b (int a,char [] cs)
	{
		return a + 2;
	}
	public static int a(int a, int b) 
	{
		return a + b;
	}
	static 
	{
		System.loadLibrary("Test");
	}
	public static native int test(int a);
	public static void main (String [] args)
	{
		//char [] cs = new char[10];
		//Test [] ts = new Test[2];
		//ts[0] = new Test();
		Test [][][][] ts = new Test[4][][][];
		int c = a(test(12), 4);
		System.out.println("hello world");
		//ts[0].a(2,3);
	}
}
