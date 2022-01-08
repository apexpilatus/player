import jakarta.servlet.http.HttpServlet;
import jakarta.servlet.http.HttpServletRequest;
import jakarta.servlet.http.HttpServletResponse;

import java.io.File;
import java.io.IOException;
import java.util.Arrays;
import java.util.Objects;

public class Lister extends HttpServlet {
    String musicPath = "/home/disk/music";

    @Override
    protected void doGet(HttpServletRequest req, HttpServletResponse resp) throws IOException {
        File musicDir = new File(musicPath);
        if (musicDir.exists() && musicDir.isDirectory()) {
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
