import jakarta.servlet.http.HttpServletResponse;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.RestController;

import java.io.IOException;

@RestController
public class MainPage {
    @GetMapping("/")
    void mainPage(HttpServletResponse resp) throws IOException {
        resp.setContentType("text/html");
        resp.getWriter().println("<head><meta charset=UTF-8><title>player</title>" +
                "<link rel=icon type=image/x-icon href=favicon.ico>" +
                "<link rel=apple-touch-icon href=apple-180x180.png sizes=180x180 type=image/png>" +
                "</head>");
        resp.getWriter().write("fuck");
    }
}
