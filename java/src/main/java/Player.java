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
    String trackFilePath = exeDirPath + "/player/tmp/track";

    @Override
    protected void doPost(HttpServletRequest req, HttpServletResponse resp) {
        String albumToPlay = req.getParameter("album");
        String trackToPlay = req.getParameter("track");
        String pictureName = "picture.jpeg";
        if (albumToPlay != null) {
            String pictureDirPath = exeDirPath + "/player/tmp";
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
                    if (file.delete()) {
                        pictureName = new Date().getTime() + ".jpeg";
                    }
                }
            }
            try (
                    BufferedWriter albumFileWriter = new BufferedWriter(new FileWriter(albumFilePath));
                    BufferedWriter trackFileWriter = new BufferedWriter(new FileWriter(trackFilePath));
                    FileInputStream flacIs = new FileInputStream(albumToPlay + "/01.flac");
                    FileOutputStream pictureOs = new FileOutputStream(pictureDirPath + "/" + pictureName)) {
                trackFileWriter.write(trackToPlay == null ? "01.flac" : trackToPlay);
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
        resp.setContentType("text/plain");
        try {
            resp.getWriter().println(pictureName);
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

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
        resp.setContentType("text/html");
        try {
            resp.getWriter().println("<head><meta charset=UTF-8><title>player</title>" +
                    "<link rel=apple-touch-icon href=apple-180x180.png sizes=180x180 type=image/png>" +
                    "</head>");
            resp.getWriter().println("<body style=background-color:gray>");
            resp.getWriter().println("<script src=play.js></script>");
            resp.getWriter().println("<p id=volume style=position:fixed;top:50px;left:20px;></p>");
            resp.getWriter().println("<button type=button onclick=setvolume(\"up\") style=border-radius:50%;color:white;background-color:black;font-size:20px;position:fixed;top:150px;left:20px;>up</button>");
            resp.getWriter().println("<button type=button onclick=setvolume(\"down\") style=border-radius:50%;color:white;background-color:black;font-size:20px;position:fixed;top:200px;left:20px;>dw</button>");
            resp.getWriter().println("<img id=picture src=" + pictureName + " style=width:200px;height:200px;position:fixed;top:300px;left:20px;>");
            resp.getWriter().println("<iframe id=tracks height=400 style=position:fixed;top:150px;right:20px;></iframe>");
        } catch (IOException e) {
            e.printStackTrace();
        }
        albums.forEach((album, albumPath) -> {
            try {
                resp.getWriter().println("<p style=text-align:center;color:black;font-size:150%; onclick=gettracks(\"" + (albumPath + "/" + album).replace(" ", "&") + "\")><b>" +
                        album.replace("fuckingslash", "/").replace("fuckingblackstar", "&#9733").replace(" anD ", " & ").replace("___", " </b><small style=color:white;>") +
                        "</b></small></p>");
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
