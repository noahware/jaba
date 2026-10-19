public class Target
{
	public int add(int a, int b)
	{
		return a + b;
	}

	public static void main(String[] args)
	{
		System.load(args[0]);

		Target t = new Target();

		// warm up so the jit compiles add, the hook has to deoptimize it
		long sum = 0;

		for (int i = 0; i < 200000; i++)
			sum += t.add(i, 1);

		int before = t.add(2, 3);
		boolean installed = install();
		boolean sdk = sdk();
		int hooked = t.add(2, 3);
		boolean removed = uninstall();
		int after = t.add(2, 3);

		System.out.println("installed=" + installed + " removed=" + removed + " sdk=" + sdk);
		System.out.println("before=" + before + " hooked=" + hooked + " after=" + after);

		// the detour calls the original and adds 100
		if (!installed || !removed || !sdk || before != 5 || hooked != 105 || after != 5)
		{
			System.out.println("FAIL");
			System.exit(1);
		}

		System.out.println("PASS");
	}

	static native boolean install();

	static native boolean sdk();

	static native boolean uninstall();
}
