package controllers;

import jakarta.servlet.http.HttpServletResponse;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.RestController;

import java.io.BufferedReader;
import java.io.FileReader;
import java.io.IOException;

@RestController
public class Power {
    @GetMapping("/power")
    void getPower(HttpServletResponse resp) throws IOException {
        resp.setContentType("text/plain");
        resp.setCharacterEncoding("utf-8");
        resp.setHeader("Cache-Control", "no-cache, no-store");
        resp.setHeader("X-Content-Type-Options", "nosniff");
        String pow = "";
        try (BufferedReader file = new BufferedReader(new FileReader("/sys/class/power_supply/BAT0/capacity"))) {
            pow = file.readLine();
        } catch (IOException ignore) {

        }
        resp.getWriter().write(pow);
    }
}
