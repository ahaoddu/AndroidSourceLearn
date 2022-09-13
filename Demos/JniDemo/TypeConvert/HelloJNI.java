
public class HelloJNI {  
   static {
      System.loadLibrary("hello");                                  
   }
 
   private native void sayHello();
   private native String sayHello(String msg);
   private native double average(int n1, int n2);
   private native double[] sumAndAverage(int[] numbers);
 
   public static void main(String[] args) {
      HelloJNI helloJni = new HelloJNI();
      helloJni.sayHello();
      helloJni.sayHello("Hello World");
      System.out.println(helloJni.average(3,5));
      int[] nums = {1,2,3};
      double[] results = helloJni.sumAndAverage(nums);
      System.out.println("sum is " + results[0] + ", average is " + results[1]);
   }
}