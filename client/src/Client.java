import java.io.*;
import java.net.*;

public class Client {
    private String server_ip;
    private String client_ip;
    private int link_port;
    private int trans_port;

    private CMDTYPE cmdType; // cmd type

    private Socket linkSocket = null;
    private Socket transSocket = null;

    public Client(String server_ip, int link_port) {
        this.server_ip = server_ip;
        this.link_port = link_port;
    }

    // get the len of a byte array
    int getBytesLen(byte[] bs) {
        int len = 0;
        for (byte b : bs) {
            if (b == 0) {
                break;
            }
            len++;
        }
        return len;
    }

    // send instructions to the server
    private boolean SendCmd(String cmd) throws IOException {
        // max cmd len = 50
        final int MAXINSTRUCTIONLEN = 50;
        if (cmd.length() > MAXINSTRUCTIONLEN) {
            System.out.println("Too long instructions, only choose the prefix string(len=" + MAXINSTRUCTIONLEN + ").");
            cmd = cmd.substring(0, 50) + "\r\n";
        } else {
            cmd = cmd + "\r\n";
        }
        // cmd cannot be empty
        if (cmd.length() == 0) {
            System.out.println("Cannot send empty cmd to the server.");
            return false;
        }
        byte[] bstream = cmd.getBytes();
        OutputStream os = this.linkSocket.getOutputStream();
        os.write(bstream);
        os.flush();
        return true;
    }

    private void setCmdType(String cmd) {
        if (cmd.length() == 0) {
            this.cmdType = CMDTYPE.ERROR;
            return;
        }
        String type = cmd.split(" ")[0];
        if (type == "USER") {
            this.cmdType = CMDTYPE.USER;
        } else if (type == "PASS") {
            this.cmdType = CMDTYPE.PASS;
        } else if (type == "RETR") {
            this.cmdType = CMDTYPE.RETR;
        } else if (type == "STOR") {
            this.cmdType = CMDTYPE.STOR;
        } else if (type == "QUIT") {
            this.cmdType = CMDTYPE.QUIT;
        } else if (type == "SYST") {
            this.cmdType = CMDTYPE.SYST;
        } else if (type == "TYPE") {
            this.cmdType = CMDTYPE.TYPE;
        } else if (type == "PORT") {
            this.cmdType = CMDTYPE.PORT;
        } else if (type == "PASV") {
            this.cmdType = CMDTYPE.PASV;
        } else if (type == "MKD") {
            this.cmdType = CMDTYPE.MKD;
        } else if (type == "CWD") {
            this.cmdType = CMDTYPE.CWD;
        } else if (type == "PWD") {
            this.cmdType = CMDTYPE.PWD;
        } else if (type == "LIST") {
            this.cmdType = CMDTYPE.LIST;
        } else if (type == "RMD") {
            this.cmdType = CMDTYPE.RMD;
        } else if (type == "RNFR") {
            this.cmdType = CMDTYPE.RNFR;
        } else if (type == "RNTO") {
            this.cmdType = CMDTYPE.RNTO;
        } else {
            this.cmdType = CMDTYPE.ERROR;
        }
    }

    private String getData() throws IOException {
        InputStream is = this.linkSocket.getInputStream();
        byte[] bstream = new byte[1024];
        is.read(bstream);
        return new String(bstream).substring(0, getBytesLen(bstream));
    }

    private void processPORT(String cmd) {
        String data = cmd.split(" ")[1];
        if (data.length() > 0) {
            String[] info = data.split(",");
            if (info.length == 6) {
                this.client_ip = info[0] + "." + info[1] + "." + info[2] + "." + info[3];
                this.trans_port = 256 * Integer.valueOf(info[4]) + Integer.valueOf(info[5]);
            }
        }
    }

    private void processPASV(String cmd) {
        // do nothing
    }

    // process data based on your cmdType and data contents
    private void processRouter(String cmd) {
        if (this.cmdType == CMDTYPE.PORT) {
            this.processPORT(cmd);
        } else if (this.cmdType == CMDTYPE.PASV) {
            this.processPASV(cmd);
        }
    }

    public void Loop() throws IOException {
        this.linkSocket = new Socket(this.server_ip, this.link_port);
        System.out.println(this.getData());
        BufferedReader inFromUser = new BufferedReader(new InputStreamReader(System.in));
        while (true) {
            // get the cmd from user
            String cmd = inFromUser.readLine();
            // set the type of the cmd
            this.setCmdType(cmd);
            // send cmd to the server
            boolean sendOver = this.SendCmd(cmd);
            // send over successfully
            if (sendOver) {
                // process
                this.processRouter(cmd);
                // get response from the server
                String data = this.getData();
                System.out.println(data);
            }
        }
    }
}

enum CMDTYPE {
    USER, PASS, RETR, STOR, QUIT, SYST, TYPE, PORT, PASV, MKD, CWD, PWD, LIST, RMD, RNFR, RNTO, ERROR
};