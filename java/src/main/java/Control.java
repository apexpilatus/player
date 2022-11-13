import jakarta.servlet.http.HttpServlet;
import jakarta.servlet.http.HttpServletRequest;
import jakarta.servlet.http.HttpServletResponse;

import java.io.*;

public class Control extends HttpServlet {
    String volumeFilePath = "/home/exe/player/tmp/volume";

    @Override
    protected void doPost(HttpServletRequest req, HttpServletResponse resp) {
        String volChangeDirection = req.getParameter("volume");
        File volFile = new File(volumeFilePath);
        if (volChangeDirection != null) {
            int vol = 5;
            if (volFile.exists()) {
                try (BufferedReader volumeFileReader = new BufferedReader(new FileReader(volumeFilePath))) {
                    vol = volumeFileReader.read();
                } catch (IOException e) {
                    e.printStackTrace();
                }
            }
            switch (volChangeDirection) {
                case "up" -> vol++;
                case "down" -> vol--;
                default -> {
                }
            }
            resp.setContentType("text/plain");
            try (BufferedWriter albumFileWriter = new BufferedWriter(new FileWriter(volumeFilePath))) {
                albumFileWriter.write(vol);
                resp.getWriter().println(vol);
            } catch (IOException e) {
                e.printStackTrace();
            }
        }
    }
    
    @Override
    protected void doGet(HttpServletRequest req, HttpServletResponse resp) {
        resp.setContentType("text/html");
        try {
            resp.getWriter().println("<head><meta charset=UTF-8></head>");
            resp.getWriter().println("<body style=background-color:gray>");
        } catch (IOException e) {
            e.printStackTrace();
        }
    }
}
