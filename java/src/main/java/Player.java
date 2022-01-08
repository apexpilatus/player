import jakarta.servlet.http.HttpServlet;
import jakarta.servlet.http.HttpServletRequest;
import jakarta.servlet.http.HttpServletResponse;

import java.io.BufferedWriter;
import java.io.File;
import java.io.FileWriter;
import java.io.IOException;

public class Player extends HttpServlet {
    String musicPath = "/home/disk/music";
    String playFilePath = "/home/sd/player/play";
    String albumFilePath = "/home/sd/player/album";

    @Override
    protected void doGet(HttpServletRequest req, HttpServletResponse resp) {
        
        File musicDir = new File(musicPath);
        if (musicDir.exists() && musicDir.isDirectory()) {
            String album = req.getParameter("album");
            if (album != null) {
                try (
                        BufferedWriter playFileWriter = new BufferedWriter(new FileWriter(new File(playFilePath)));
                        BufferedWriter albumFileWriter = new BufferedWriter(new FileWriter(new File(albumFilePath)));
                ) {
                    playFileWriter.write(1);
                    albumFileWriter.write(album);
                } catch (IOException e) {
                    e.printStackTrace();
                }
            }
            String[] albums = musicDir.list();
            Arrays.sort(albums);
            if (Objects.requireNonNull(albums).length != 0) {
                resp.setContentType("text/html");
                for (String album:albums){
                    resp.getWriter().println("<p style=\"font-size:1em\"><a href=http://jetson:8080/player?album=" +
                                             musicPath + "/" + album.replace(" ","%20") + ">" + "<b>" +
                                             album.replace("fuckingslash","/").
                                             replace("fuckingquestion","?").
                                             replace("fuckingblackstar","&#9733").
                                             replace("___","</b> ")+
                                             "</b></a></p>");
                }
                resp.getWriter().println("<p style=\"font-size:1em\">_ _ _ _ _ _ _</p>");
            }
        }
    }
}
