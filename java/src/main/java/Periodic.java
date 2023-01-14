import jakarta.servlet.http.HttpServlet;
import jakarta.servlet.http.HttpServletRequest;
import jakarta.servlet.http.HttpServletResponse;
import org.jflac.FLACDecoder;
import org.jflac.metadata.Metadata;
import org.jflac.metadata.Picture;

import java.io.*;
import java.lang.reflect.Field;
import java.net.InetSocketAddress;
import java.net.Socket;
import java.util.Base64;

public class Periodic extends HttpServlet implements Common {
    private synchronized int action2SwitchDevice() {
        int ret = -1;
        currentPlayer[0] = "none";
        for (String playerHost : playerHosts) {
            try (Socket sock = new Socket()) {
                sock.connect(new InetSocketAddress(playerHost, playerPort), timeOut);
                sock.setSoTimeout(timeOut);
                BufferedWriter writer = new BufferedWriter(new OutputStreamWriter(sock.getOutputStream()));
                BufferedReader reader = new BufferedReader(new InputStreamReader(sock.getInputStream()));
                byte op = 2;
                writer.write(op);
                writer.flush();
                ret = reader.read();
                currentPlayer[0] = playerHost;
                break;
            } catch (IOException ignored) {
            }
        }
        return ret;
    }

    @Override
    protected void doPost(HttpServletRequest req, HttpServletResponse resp) {
        int vol = action2SwitchDevice();
        resp.setContentType("text/plain");
        try {
            resp.getWriter().println(currentPlayer[0] + " " + vol);
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    @Override
    protected void doGet(HttpServletRequest req, HttpServletResponse resp) {
        String albumToShow = req.getParameter("album");
        if (albumToShow != null) {
            resp.setContentType("image/jpeg");
            byte[] pictureBytes = {1, 2, 3};
            try (FileInputStream flacIs = new FileInputStream(albumToShow + "/01.flac")) {
                FLACDecoder flacDec = new FLACDecoder(flacIs);
                Metadata[] metas = flacDec.readMetadata();
                for (Metadata meta : metas) {
                    if (meta.toString().contains("Picture")) {
                        Picture picMeta = (Picture) meta;
                        Class<? extends Picture> c = picMeta.getClass();
                        Field f = c.getDeclaredField("image");
                        f.setAccessible(true);
                        pictureBytes = (byte[]) f.get(picMeta);
                    }
                }
                resp.getOutputStream().write(Base64.getEncoder().encode(pictureBytes));
            } catch (Exception e) {
                e.printStackTrace();
            }
        }
    }

}
