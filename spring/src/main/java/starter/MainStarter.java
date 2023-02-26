package starter;

import controllers.MainPage;
import controllers.Volume;
import org.springframework.boot.SpringApplication;
import org.springframework.boot.autoconfigure.SpringBootApplication;
import org.springframework.context.annotation.Import;

@SpringBootApplication
@Import({MainPage.class, Volume.class})
public class MainStarter {
    public static void main(String[] args) {
        SpringApplication.run(MainStarter.class, args);
    }
}
