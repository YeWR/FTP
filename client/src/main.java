import java.io.IOException;

public class main {
    public static void main(String[] args) throws IOException, InterruptedException {
        String ip = "127.0.0.1";//"192.168.6.107";//192.168.56.1
        int link_port = 6789;
        Client client = new Client(ip, link_port);
        client.Loop();
    }
}
