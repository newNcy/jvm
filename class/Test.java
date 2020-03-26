

class Test
{
	int a = 0;
	public int b (int a)
	{
		return a + 2;
	}
	public int a(int a, int b) 
	{
		return a + b;
	}
	public static void main (String [] args)
	{
		char [] cs = new char[10];
		Test [] ts = new Test[2];
		ts[0] = new Test();
		System.out.println("hello world");
		ts[0].a(2,3);
	}
}
