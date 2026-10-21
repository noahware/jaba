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

		// the hook has to survive the jit: every call goes through the detour (+100) and the original it calls
		// gets compiled, a caller that inlined add or a recursing original would break the sum
		long hot = 0;

		for (int i = 0; i < 2000000; i++)
			hot += t.add(i, 1);

		boolean removed = uninstall();
		int after = t.add(2, 3);

		System.out.println("installed=" + installed + " removed=" + removed + " sdk=" + sdk);
		System.out.println("before=" + before + " hooked=" + hooked + " after=" + after);

		// the detour calls the original and adds 100
		long hot_expected = 2000000L * 2000001L / 2 + 100L * 2000000L;

		System.out.println("hot=" + hot + " expected=" + hot_expected);

		if (!installed || !removed || !sdk || before != 5 || hooked != 105 || after != 5 || hot != hot_expected)
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
