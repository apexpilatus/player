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
        resp.getWriter().write("fuck");
    }
}
