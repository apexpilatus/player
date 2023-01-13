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
import java.util.*;

public class Player extends HttpServlet {
    String[] musicDirPaths = {"/home/store/music/qbz", "/home/store/music/dzr", "/home/store/music/hack/1", "/home/store/music/hack/2", "/home/store/music/hack/3", "/home/store/music/hack/4"};
    String exeDirPath = "/home/exe";
    String[] playerHosts = {"wilkins"};
    int playerPort = 8888;
    int timeOut = 4000;

    private void action0Play(String albumToPlay, String trackToPlay) {
        for (String playerHost : playerHosts) {
            try (Socket sock = new Socket();
                 BufferedWriter sockWriter = new BufferedWriter(new OutputStreamWriter(sock.getOutputStream()));
                 BufferedReader sockReader = new BufferedReader(new InputStreamReader(sock.getInputStream()))) {
                sock.connect(new InetSocketAddress(playerHost, playerPort), timeOut);
                sock.setSoTimeout(timeOut);
                byte op = 0;
                sockWriter.write(op);
                sockWriter.flush();
                sockReader.readLine();
                sockWriter.write(albumToPlay);
                sockWriter.flush();
                sockReader.readLine();
                sockWriter.write(trackToPlay == null ? "01.flac" : trackToPlay);
                sockWriter.flush();
                sockReader.readLine();
                break;
            } catch (Exception ignored) {
            }
        }
    }

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
            try (FileInputStream flacIs = new FileInputStream(albumToPlay + "/01.flac");
                 FileOutputStream pictureOs = new FileOutputStream(pictureDirPath + "/" + pictureName)) {
                action0Play(albumToPlay, trackToPlay);
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
            return;
        }

        Map<String, List<String>> albums = new TreeMap<>();
        for (String musicDirPath : musicDirPaths) {
            File musicDir = new File(musicDirPath);
            if (musicDir.exists()) {
                for (String album : Objects.requireNonNull(musicDir.list())) {
                    albums.computeIfAbsent(album, (k) -> new ArrayList<>()).add(musicDirPath);
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
            resp.getWriter().println("<body style=background-color:slategray>");
            resp.getWriter().println("<script src=main.js></script>");
            resp.getWriter().println("<script>setvolume(\"keep\")</script>");
            resp.getWriter().println("<p id=volume style=position:fixed;top:80px;left:20px;font-size:20px;></p>");
            resp.getWriter().println("<button type=button onclick=setvolume(\"up\") style=border-radius:20px;color:black;background-color:white;font-size:20px;position:fixed;top:50px;left:20px;>up</button>");
            resp.getWriter().println("<button type=button onclick=setvolume(\"down\") style=border-radius:20px;color:black;background-color:white;font-size:20px;position:fixed;top:150px;left:20px;>dw</button>");
            resp.getWriter().println("<img hidden id=picturebytes style=width:115px;height:115px;position:fixed;top:10px;right:480px;border-style:solid;>");
            resp.getWriter().println("<iframe hidden id=tracks width=450 height=430 style=\"position:fixed;top:10px;right:10px;border-style:solid;\"></iframe>");
            resp.getWriter().println("<button hidden type=button id=hidebutton onclick=hideTracks() style=border-radius:20%;color:red;background-color:black;font-size:25px;position:fixed;top:460px;right:10px;>X</button>");
            resp.getWriter().println("<img id=picture src=" + pictureName + " onclick=makebig() style=width:250px;height:250px;position:fixed;top:300px;left:5px;border-style:solid;border-color:white;>");
            resp.getWriter().println("<ul style=padding-left:271px;font-size:150%;line-height:180%;list-style-type:circle;>");
        } catch (IOException e) {
            e.printStackTrace();
        }
        albums.forEach((album, albumPathList) -> albumPathList.forEach((albumPath) -> {
            try {
                resp.getWriter().println("<li><b style=color:black; onclick=gettracks(\"" + (albumPath + "/" + album).replace(" ", "&") + "\")>" +
                        album.replace("fuckingslash", "/").replace("fuckingblackstar", "&#9733").replace("fuckingplus", "&#43").replace(" anD ", " & ").replace("___", " <small style=color:white;>") +
                        "</li></b></small>");
            } catch (IOException e) {
                e.printStackTrace();
            }
        }));
        try {
            resp.getWriter().println("</ul>");
            resp.getWriter().println("</body>");
        } catch (IOException e) {
            e.printStackTrace();
        }
    }
}
