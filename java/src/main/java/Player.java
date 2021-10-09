import jakarta.servlet.http.HttpServlet;
import jakarta.servlet.http.HttpServletRequest;
import jakarta.servlet.http.HttpServletResponse;

import java.io.BufferedWriter;
import java.io.File;
import java.io.FileWriter;
import java.io.IOException;

public class Player extends HttpServlet {
    String playFilePath = "/home/sd/player/play";
    String albumFilePath = "/home/sd/player/album";

    @Override
    protected void doGet(HttpServletRequest req, HttpServletResponse resp) {
        String album = req.getParameter("album");
        if (album != null) {
            try (
                    BufferedWriter playFileWriter = new BufferedWriter(new FileWriter(new File(playFilePath)));
                    BufferedWriter albumFileWriter = new BufferedWriter(new FileWriter(new File(albumFilePath)));
            ) {
                playFileWriter.write(1);
                albumFileWriter.write(album);
                resp.sendRedirect("http://jetson:8080/lister");
            } catch (IOException e) {
                e.printStackTrace();
            }
        }
    }
}
