import jakarta.servlet.http.HttpServlet;
import jakarta.servlet.http.HttpServletRequest;
import jakarta.servlet.http.HttpServletResponse;

import java.io.BufferedWriter;
import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.util.Arrays;
import java.util.Objects;

public class Player extends HttpServlet {
    String musicPath = "/home/store/music";
    String albumFilePath = "/home/store/player/album";

    @Override
    protected void doGet(HttpServletRequest req, HttpServletResponse resp) {
        File musicDir = new File(musicPath);
        if (musicDir.exists() && musicDir.isDirectory()) {
            String albumToPlay = req.getParameter("album");
            if (albumToPlay != null) {
                try (
                        BufferedWriter playFileWriter = new BufferedWriter(new FileWriter(new File(playFilePath)));
                        BufferedWriter albumFileWriter = new BufferedWriter(new FileWriter(new File(albumFilePath)));
                ) {
                    playFileWriter.write(1);
                    albumFileWriter.write(albumToPlay);
                } catch (IOException e) {
                    e.printStackTrace();
                }
            }
            String[] albums = musicDir.list();
            Arrays.sort(albums);
            if (Objects.requireNonNull(albums).length != 0) {
                resp.setContentType("text/html");
                try {
                    resp.getWriter().println("<body style=\"background-color:gray\">");
                    for (String album:albums){
                        resp.getWriter().println("<p style=\"text-align:center;color:white;font-size:150%\"><a href=http://asus:8080/player?album=" +
                                                 musicPath + "/" + album.replace(" ","%20") + ">" +
                                                 album.replace("fuckingslash","/").
                                                 replace("fuckingblackstar","&#9733").
                                                 replace(" anD "," & ").
                                                 replace("___","</a> <b><small>") +
                                                 "</a></b></small></p>");
                    }
                    resp.getWriter().println("<p style=\"font-size:1em;text-align:center\">_ _ _ _ _ _ _</p>");
                    resp.getWriter().println("</body>");
                } catch (IOException e) {
                    e.printStackTrace();
                }
            }
        }
    }
}
