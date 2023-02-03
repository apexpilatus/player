import jakarta.servlet.http.HttpServlet;
import jakarta.servlet.http.HttpServletRequest;
import jakarta.servlet.http.HttpServletResponse;
import org.jflac.FLACDecoder;
import org.jflac.metadata.Metadata;
import org.jflac.metadata.Picture;

import java.io.FileInputStream;
import java.io.IOException;
import java.lang.reflect.Field;
import java.util.Base64;

public class Periodic extends HttpServlet {

    @Override
    protected void doPost(HttpServletRequest req, HttpServletResponse resp) {
        Common.IpcActions ipcActions = new Common.IpcActions();
        int vol = ipcActions.action2SwitchDevice();
        resp.setContentType("text/plain");
        try {
            resp.getWriter().println(ipcActions.getCurrentPlayer() + " " + vol);
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
