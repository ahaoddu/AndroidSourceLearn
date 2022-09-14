
public class HelloJNI {  
   static {
      System.loadLibrary("hello");                                  
   }

   private int number = 88;
   private static double numberStatic = 1998.2;
   private String message = "Hello from Java";
   
   // //实例方法
   // private String instanceMethod() {
   //    return "Instance Method";
   // }
   // //静态方法
   // private static String staticMethod() {
   //    return "Static Method";
   // }
 
   private native void sayHello();

   //基本类型
   private native double average(int n1, int n2);

   //String 类型
   private native String sayHello(String msg);

   //数组
   private native double[] sumAndAverage(int[] numbers);
   
   //访问对象
   private native void modifyVariable();

   // private native void callJavaMethod();

   
   public static void main(String[] args) {
      HelloJNI helloJni = new HelloJNI();

      helloJni.sayHello();

      String result = helloJni.sayHello("Hello World");
      System.out.println("sayHello result is " + result);

      System.out.println(helloJni.average(3,5));

      int[] nums = {1,2,3};
      double[] results = helloJni.sumAndAverage(nums);
      System.out.println("sum is " + results[0] + ", average is " + results[1]);

      helloJni.modifyVariable();

      System.out.println("in java the message is " + helloJni.message);
   } 
}