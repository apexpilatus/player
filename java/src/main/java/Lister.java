import jakarta.servlet.http.HttpServlet;
import jakarta.servlet.http.HttpServletRequest;
import jakarta.servlet.http.HttpServletResponse;

import java.io.File;
import java.io.IOException;
import java.util.Arrays;
import java.util.Objects;

public class Lister extends HttpServlet {
    String musicPath = "/home/disk/music";
    File musicDir = new File(musicPath);

    @Override
    protected void doGet(HttpServletRequest req, HttpServletResponse resp) throws IOException {
        if (musicDir.exists() && musicDir.isDirectory()) {
            String[] albums = musicDir.list();
            Arrays.sort(albums);
            if (Objects.requireNonNull(albums).length != 0) {
                resp.setContentType("text/html");
                for (String album:albums){
                    resp.getWriter().println("<p style=\"font-size:2em\"><a href=http://jetson:8080/player?album=" + album.replace(" ","%20") + ">" + album + "</a></p>");
                }
                resp.getWriter().println("<p style=\"font-size:3em\">_ _ _ _ _ _ _</p>");
            }
        }
    }
}
