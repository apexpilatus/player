package controllers;

import jakarta.servlet.http.HttpServletResponse;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.RestController;

import java.io.IOException;

@RestController
public class MainPage {
    @GetMapping("/")
    void mainPage(HttpServletResponse resp) throws IOException {
        resp.setContentType("text/html");
        resp.getWriter().println("<head><meta charset=UTF-8><title>player</title>");
        resp.getWriter().println("<link rel=apple-touch-icon href=apple-180x180.png sizes=180x180 type=image/png>");
        resp.getWriter().println("<script>function setvolume(direction){");
        resp.getWriter().println("\tconst xhttp = new XMLHttpRequest();");
        resp.getWriter().println("\txhttp.onload = function() {");
        resp.getWriter().println("\t\tdocument.getElementById(\"volume\").innerHTML = \"&#127911 \" + this.responseText;");
        resp.getWriter().println("\t}");
        resp.getWriter().println("\txhttp.open(\"GET\", window.location.href + \"volume\" + \"?direction=\" + direction);");
        resp.getWriter().println("\txhttp.send();");
        resp.getWriter().println("}</script>");
        resp.getWriter().println("</head>");
        resp.getWriter().println("<body style=background-color:slategray>");
        resp.getWriter().println("<p id=volume style=position:fixed;top:80px;left:20px;font-size:20px;></p>");
        resp.getWriter().println("<script>setvolume(\"init\")</script>");
        resp.getWriter().println("</body>");
    }
}
