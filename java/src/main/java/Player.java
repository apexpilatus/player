import jakarta.servlet.http.HttpServlet;
import jakarta.servlet.http.HttpServletRequest;
import jakarta.servlet.http.HttpServletResponse;
import org.jflac.FLACDecoder;
import org.jflac.metadata.Metadata;
import org.jflac.metadata.Picture;

import java.io.*;
import java.lang.reflect.Field;
import java.util.*;

public class Player extends HttpServlet {
    String[] musicDirPaths = {"/home/store/music", "/home/disk/music"};
    String exeDirPath = "/home/exe";
    String albumFilePath = exeDirPath + "/player/tmp/album";

    @Override
    protected void doGet(HttpServletRequest req, HttpServletResponse resp) {
        Map<String, String> albums = new TreeMap<>();
        for (String musicDirPath : musicDirPaths) {
            File musicDir = new File(musicDirPath);
            if (musicDir.exists()) {
                for (String album : Objects.requireNonNull(musicDir.list())) {
                    albums.put(album, musicDirPath);
                }
            }
        }
        if (!albums.isEmpty()) {
            String pictureDirPath = exeDirPath + "/player/tmp";
            String pictureName = "picture.jpeg";
            File dirFile = new File(exeDirPath);
            for (String file : Objects.requireNonNull(dirFile.list())) {
                if (file.contains("apache-tom")) {
                    pictureDirPath = exeDirPath + "/" + file + "/" + "webapps/ROOT";
                }
            }
            dirFile = new File(pictureDirPath);
            for (File file : Objects.requireNonNull(dirFile.listFiles())) {
                if (file.getName().contains("jpeg")) {
                    pictureName = file.getName();
                }
            }
            String albumToPlay = req.getParameter("album");
            if (albumToPlay != null) {
                for (File file : Objects.requireNonNull(dirFile.listFiles())) {
                    if (file.getName().contains("jpeg")) {
                        if (file.delete()) {
                            pictureName = new Date().getTime() + ".jpeg";
                        }
                    }
                }
                try (
                        BufferedWriter albumFileWriter = new BufferedWriter(new FileWriter(albumFilePath));
                        FileInputStream flacIs = new FileInputStream(albumToPlay + "/01.flac");
                        FileOutputStream pictureOs = new FileOutputStream(pictureDirPath + "/" + pictureName)) {
                    albumFileWriter.write(albumToPlay);
                    FLACDecoder flacDec = new FLACDecoder(flacIs);
                    Metadata[] metas = flacDec.readMetadata();
                    for (Metadata meta : metas) {
                        if (meta.toString().contains("Picture")) {
                            Picture picMeta = (Picture) meta;
                            Class<? extends Picture> c = picMeta.getClass();
                            Field f = c.getDeclaredField("image");
                            f.setAccessible(true);
                            byte[] picture = (byte[]) f.get(picMeta);
                            pictureOs.write(picture);
                        }
                    }
                } catch (Exception e) {
                    e.printStackTrace();
                }
            }
            resp.setContentType("text/html");
            try {
                resp.getWriter().println("<head><meta charset=UTF-8><title>player</title><link rel=apple-touch-icon href=apple-touch-icon.png type=image/png></head>");
                resp.getWriter().println("<body style=background-color:gray>");
                resp.getWriter().println("<script src=volume.js></script>");
                resp.getWriter().println("<p id=volume style=position:fixed;top:50px;left:20px;></p>");
                resp.getWriter().println("<button type=button onclick=volume(\"up\") style=position:fixed;top:150px;left:20px;>up</button>");
                resp.getWriter().println("<button type=button onclick=volume(\"down\") style=position:fixed;top:200px;left:20px;>dw</button>");
                resp.getWriter().println("<img src=" + pictureName + " style=width:200px;height:200px;position:fixed;top:300px;left:20px;>");
            } catch (IOException e) {
                e.printStackTrace();
            }
            albums.forEach((album, albumPath) -> {
                try {
                    resp.getWriter().println("<p style=text-align:center;color:white;font-size:150%><a href=http://" + req.getHeader("Host") +
                            "/player?album=" + albumPath + "/" + album.replace(" ", "%20") + ">" +
                            album.replace("fuckingslash", "/").replace("fuckingblackstar", "&#9733").replace(" anD ", " & ").replace("___", "</a> <b><small>") +
                            "</a></b></small></p>");
                } catch (IOException e) {
                    e.printStackTrace();
                }
            });
            try {
                resp.getWriter().println("<p style=font-size:1em;text-align:center>_ _ _ _ _ _ _</p>");
                resp.getWriter().println("</body>");
            } catch (IOException e) {
                e.printStackTrace();
            }
        }
    }
}
