from django.apps import AppConfig
import threading

class PrisesConfig(AppConfig):
    default_auto_field = 'django.db.models.BigAutoField'
    name = 'prises'

    def ready(self):
        from . import mqtt_wow

        mqtt_thread = threading.Thread(target=mqtt_wow.mqtt_loop)
        mqtt_thread.daemon = True
        mqtt_thread.start()