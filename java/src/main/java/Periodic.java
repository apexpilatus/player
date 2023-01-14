import jakarta.servlet.http.HttpServlet;
import jakarta.servlet.http.HttpServletRequest;
import jakarta.servlet.http.HttpServletResponse;

import java.io.IOException;
import java.util.Objects;

public class Periodic extends HttpServlet implements Common{
    static int vol = 0;
    @Override
    protected void doPost(HttpServletRequest req, HttpServletResponse resp) {
        vol++;
        resp.setContentType("text/plain");
        try {
            resp.getWriter().println(currentPlayer[0] + " " + vol);
        } catch (IOException e) {
            e.printStackTrace();
        }
    }
}
