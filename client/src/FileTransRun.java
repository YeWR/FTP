import java.io.*;
import java.net.InetSocketAddress;
import java.net.ServerSocket;
import java.net.Socket;

public class FileTransRun extends Thread {

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

    public void setOver(boolean isOver) {
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

    private void LIST(Socket transSocket) {
        DataInputStream in = null;
        try {
            in = new DataInputStream(new BufferedInputStream(transSocket.getInputStream()));
        } catch (IOException e) {
            e.printStackTrace();

        }
        byte[] bytes = new byte[16 * 1024];
        int count = -1;
        while (true) {
            try {
                assert in != null;
                count = in.read(bytes);
            } catch (IOException e) {
                e.printStackTrace();
            }
            if (count > 0) {
                String list = new String(bytes).substring(0, getBytesLen(bytes));
                System.out.println(list);
            } else {
                break;
            }
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

    private void RETR(Socket transSocket) {
        DataInputStream in = null;
        try {
            in = new DataInputStream(new BufferedInputStream(transSocket.getInputStream()));
        } catch (IOException e) {
            e.printStackTrace();

        }
        OutputStream out = null;
        try {
            out = new FileOutputStream(this.fileName);
        } catch (FileNotFoundException e) {
            System.out.println("cannot open file " + this.fileName);

            e.printStackTrace();
        }
        byte[] bytes = new byte[16 * 1024];

        int count = 1;
        while (count > 0) {
            try {
                assert in != null;
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
        FileInputStream in = null;
        try {
            in = new FileInputStream(this.fileName);
        } catch (FileNotFoundException e) {
            e.printStackTrace();
        }

        OutputStream out = null;

        try {
            out = transSocket.getOutputStream();
        } catch (IOException e) {
            e.printStackTrace();
        }

        byte[] bytes = new byte[16 * 1024];

        int count = 1;
        while (count > 0) {
            try {
                assert in != null;
                count = in.read(bytes);
            } catch (IOException e) {
                e.printStackTrace();
            }
            try {
                assert out != null;
                out.write(bytes);
            } catch (IOException e) {
                e.printStackTrace();
            }
        }
        try {
            out.flush();
        } catch (IOException e) {
            e.printStackTrace();
        }

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

        String data = null;
        try {
            data = this.getData(this.cmdSocket);
        } catch (IOException e) {
            e.printStackTrace();
        }
        System.out.println(data);
        this.isOver = true;
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
                assert serverSocket != null;
                transSocket = serverSocket.accept();
            } catch (IOException e) {
                String data = null;
                try {
                    data = this.getData(this.cmdSocket);
                } catch (IOException e1) {
                    e.printStackTrace();
                }
                try {
                    serverSocket.close();
                } catch (IOException e1) {
                    e1.printStackTrace();
                }
                System.out.println(data);
                this.isOver = true;
                return;
            }
            try {
                serverSocket.close();
            } catch (IOException e1) {
                e1.printStackTrace();
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
        if (this.direction == CMDTYPE.LIST) {
            this.LIST(transSocket);
        } else if (this.direction == CMDTYPE.RETR) {
            assert transSocket != null;
            this.RETR(transSocket);
        } else if (this.direction == CMDTYPE.STOR) {
            this.STOR(transSocket);
        }
        if (transSocket != null) {
            try {
                transSocket.close();
            } catch (IOException e) {
                e.printStackTrace();
            }
        }
    }
}

