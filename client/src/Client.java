import java.io.*;
import java.net.*;

public class Client {
    // server ip
    private String server_ip;
    // PORT ip
    private String port_ip;
    // PASV recv ip
    private String pasv_ip;
    // link port
    private int link_port;
    // PORT port
    private int port_port;
    // PASV port
    private int pasv_port;

    private CMDTYPE cmdType; // cmd type
    private CMDTYPE lastCmdType;

    private Socket linkSocket = null;
    // port socket
    private Socket transSocket = null;
    // port server socket
    private ServerSocket serverSocket = null;
    // thread
    private FileTransRun fileThread = null;

    public Client(String server_ip, int link_port) {
        this.server_ip = server_ip;
        this.link_port = link_port;
    }

    // get the len of a byte array
    private int getBytesLen(byte[] bs) {
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
        final int MAXINSTRUCTIONLEN = 100;
        if (cmd.length() > MAXINSTRUCTIONLEN) {
            System.out.println("Too long instructions, only choose the prefix string(len=" + MAXINSTRUCTIONLEN + ").");
            cmd = cmd.substring(0, MAXINSTRUCTIONLEN) + "\r\n";
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
        if (type.equals("USER")) {
            this.cmdType = CMDTYPE.USER;
        } else if (type.equals("PASS")) {
            this.cmdType = CMDTYPE.PASS;
        } else if (type.equals("RETR")) {
            this.cmdType = CMDTYPE.RETR;
        } else if (type.equals("STOR")) {
            this.cmdType = CMDTYPE.STOR;
        } else if (type.equals("QUIT")) {
            this.cmdType = CMDTYPE.QUIT;
        } else if (type.equals("SYST")) {
            this.cmdType = CMDTYPE.SYST;
        } else if (type.equals("TYPE")) {
            this.cmdType = CMDTYPE.TYPE;
        } else if (type.equals("PORT")) {
            this.cmdType = CMDTYPE.PORT;
        } else if (type.equals("PASV")) {
            this.cmdType = CMDTYPE.PASV;
        } else if (type.equals("MKD")) {
            this.cmdType = CMDTYPE.MKD;
        } else if (type.equals("CWD")) {
            this.cmdType = CMDTYPE.CWD;
        } else if (type.equals("PWD")) {
            this.cmdType = CMDTYPE.PWD;
        } else if (type.equals("LIST")) {
            this.cmdType = CMDTYPE.LIST;
        } else if (type.equals("RMD")) {
            this.cmdType = CMDTYPE.RMD;
        } else if (type.equals("RNFR")) {
            this.cmdType = CMDTYPE.RNFR;
        } else if (type.equals("RNTO")) {
            this.cmdType = CMDTYPE.RNTO;
        } else {
            this.cmdType = CMDTYPE.ERROR;
        }
    }

    private String getData(Socket socket) throws IOException {
        InputStream is = socket.getInputStream();
        byte[] bstream = new byte[16384];
        is.read(bstream);
        return new String(bstream).substring(0, getBytesLen(bstream));
    }

    private void processPORT(String cmd) {
        String data = cmd.split(" ")[1];
        if (data.length() > 0) {
            String[] info = data.split(",");
            if (info.length == 6) {
                this.port_ip = info[0] + "." + info[1] + "." + info[2] + "." + info[3];
                this.port_port = 256 * Integer.valueOf(info[4]) + Integer.valueOf(info[5]);
            }
        }
    }

    private void getPASVResponse(String response) {
        String data = response.split("\\(")[1].split("\\)")[0];
        String[] info = data.split(",");
        if (info.length == 6) {
            this.pasv_ip = info[0] + "." + info[1] + "." + info[2] + "." + info[3];
            this.pasv_port = 256 * Integer.valueOf(info[4]) + Integer.valueOf(info[5]);
        }
    }

    // process RETR
    private void processRETR(String cmd) throws IOException {
        this.serverSocket = null;
        // process file name
        String[] path = cmd.split("[/]");
        String fileName = path[path.length - 1];

        this.fileThread = null;

        if (this.lastCmdType == CMDTYPE.PORT) {
            this.fileThread = new FileTransRun(this.lastCmdType, this.cmdType, this.port_port, this.port_ip, fileName, this.linkSocket);
        } else if (this.lastCmdType == CMDTYPE.PASV) {
            this.fileThread = new FileTransRun(this.lastCmdType, this.cmdType, this.pasv_port, this.pasv_ip, fileName, this.linkSocket);
        } else {
            return;
        }
    }

    // process data based on your cmdType and data contents
    private void processRouter(String cmd) throws IOException {
        if (this.cmdType == CMDTYPE.PORT) {
            this.processPORT(cmd);
        } else if (this.cmdType == CMDTYPE.RETR) {
            this.processRETR(cmd);
        }
    }

    public void Loop() throws IOException, InterruptedException {
        this.linkSocket = new Socket(this.server_ip, this.link_port);
        System.out.println(this.getData(this.linkSocket));
        BufferedReader inFromUser = new BufferedReader(new InputStreamReader(System.in));
        while (true) {
            // get the cmd from user
            if (this.fileThread != null) {
                if (!this.fileThread.isOver()) {
                    Thread.sleep(1000);
                    continue;
                }
            }
            System.out.print("$ ");
            String cmd = inFromUser.readLine();
            // set the type of the cmd
            this.setCmdType(cmd);
            boolean sendOver = false;
            if (this.lastCmdType == CMDTYPE.PASV) {
                // send cmd to the server
                sendOver = this.SendCmd(cmd);
                this.processRouter(cmd);
            } else {
                this.processRouter(cmd);
                // send cmd to the server
                sendOver = this.SendCmd(cmd);
            }
            // send over successfully
            if (sendOver) {
                // get response from the server
                String data = this.getData(this.linkSocket);
                System.out.println(data);
                if (this.cmdType == CMDTYPE.PASV) {
                    this.getPASVResponse(data);
                }
                // if 150, then recv file
                else if(this.cmdType == CMDTYPE.RETR && this.fileThread != null){
                    String code = data.split(" ")[0];
                    if(code.equals("150")){
                        this.fileThread.start();
                    }else {
                        this.fileThread.setOver(true);
                    }
                }
            }

            this.lastCmdType = this.cmdType;
        }
        // this.linkSocket.close();
    }
}

enum CMDTYPE {
    USER, PASS, RETR, STOR, QUIT, SYST, TYPE, PORT, PASV, MKD, CWD, PWD, LIST, RMD, RNFR, RNTO, ERROR
};

class FileTransRun extends Thread {
    // type:
    private CMDTYPE type;
    // direction::
    private CMDTYPE direction;
    private String ip = null;
    private String fileName = null;
    private int port;
    private Socket cmdSocket = null;
    private boolean isOver = false;

    FileTransRun(CMDTYPE t_type, CMDTYPE t_direction, int t_port, String t_ip, String t_fileName, Socket socket) {
        this.type = t_type;
        this.direction = t_direction;
        this.port = t_port;
        this.ip = t_ip;
        fileName = t_fileName;
        cmdSocket = socket;
    }

    public boolean isOver() {
        return isOver;
    }

    public void setOver(boolean isOver){
        this.isOver = isOver;
    }

    // get the len of a byte array
    private int getBytesLen(byte[] bs) {
        int len = 0;
        for (byte b : bs) {
            if (b == 0) {
                break;
            }
            len++;
        }
        return len;
    }

    private String getData(Socket socket) throws IOException {
        InputStream is = socket.getInputStream();
        byte[] bstream = new byte[16384];
        is.read(bstream);
        return new String(bstream).substring(0, getBytesLen(bstream));
    }

    private void RETR(Socket transSocket) {
        DataInputStream in = null;
        try {
            in = new DataInputStream(new BufferedInputStream(transSocket.getInputStream()));
        } catch (IOException e) {
            e.printStackTrace();

        }
        OutputStream out = null;
        try {
            out = new FileOutputStream(fileName);
        } catch (FileNotFoundException e) {
            System.out.println("cannot open file " + fileName);

            e.printStackTrace();
        }
        byte[] bytes = new byte[16 * 1024];

        int count = 1;
        assert in != null;
        while (count > 0) {
            try {
                count = in.read(bytes);
                if (count <= 0) {
                    break;
                }
            } catch (IOException e) {
                e.printStackTrace();
                break;
            }
            assert out != null;
            try {
                out.write(bytes, 0, count);
            } catch (IOException e) {
                e.printStackTrace();
            }
        }
        assert out != null;
        try {
            out.close();
        } catch (IOException e) {
            e.printStackTrace();
        }
        try {
            in.close();
        } catch (IOException e) {
            e.printStackTrace();
        }
        try {
            transSocket.close();
        } catch (IOException e) {
            e.printStackTrace();
        }

        String data = null;
        try {
            data = this.getData(this.cmdSocket);
        } catch (IOException e) {
            e.printStackTrace();
        }
        System.out.println(data);
        this.isOver = true;
    }

    private void STOR(Socket transSocket) {

    }

    @Override
    public void run() {
        // PORT
        Socket transSocket = null;
        if (type == CMDTYPE.PORT) {
            ServerSocket serverSocket = null;
            try {
                serverSocket = new ServerSocket(this.port);
                serverSocket.setSoTimeout(5000);// 5s
            } catch (IOException e) {
                e.printStackTrace();
            }
            // waiting for server ...
            try {
                transSocket = serverSocket.accept();
            } catch (IOException e) {
                String data = null;
                try {
                    data = this.getData(this.cmdSocket);
                } catch (IOException e1) {
                    e.printStackTrace();
                }
                System.out.println(data);
                this.isOver = true;
                return;
            }
        }
        // PASV
        else if (type == CMDTYPE.PASV) {
            try {
                transSocket = new Socket();
                transSocket.connect(new InetSocketAddress(this.ip, this.port), 5000);
            } catch (IOException e) {
                String data = null;
                try {
                    data = this.getData(this.cmdSocket);
                } catch (IOException e1) {
                    e.printStackTrace();
                }
                System.out.println(data);
                this.isOver = true;
                return;
            }
        }
        if (this.direction == CMDTYPE.RETR) {
            this.RETR(transSocket);
        } else if (this.direction == CMDTYPE.STOR) {
            this.STOR(transSocket);
        }
    }
}