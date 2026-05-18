
from django.urls import path
from . import views

urlpatterns = [
    path('', views.base),
    path('controles/', views.controle_prises),
    path('switch_led/', views.switch_led),
]

