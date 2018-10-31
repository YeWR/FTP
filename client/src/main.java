import java.io.BufferedReader;
        import java.io.IOException;
        import java.io.InputStreamReader;

public class main {
    public static void main(String[] args){
        BufferedReader inFromUser = new BufferedReader(new InputStreamReader(System.in));

        System.out.println("Please input the ip of the server: ");
        String ip = null;// "192.168.6.107";//192.168.56.1
        try {
            ip = inFromUser.readLine();
        } catch (IOException e) {
            // e.printStackTrace();
        }
        System.out.println("Please input the port the server: ");
        int link_port = 0;
        try {
            link_port = Integer.valueOf(inFromUser.readLine());
        } catch (IOException e) {
            // e.printStackTrace();
        }
        catch (NumberFormatException e){

        }

        System.out.println("Waiting for connecting...");

        Client client = new Client(ip, link_port);
        client.Loop();
    }
}
