package controllers;

import jakarta.servlet.http.HttpServletResponse;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.RestController;

import java.io.IOException;

@RestController
public class AlbumPage {
    @GetMapping("/album")
    void albumPage(HttpServletResponse resp) throws IOException {
        resp.setContentType("text/html");
        resp.getWriter().println(System.getProperty("user.dir"));
    }
}