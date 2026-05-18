from django.shortcuts import render
from django.http import JsonResponse
from django.views.decorators.csrf import csrf_exempt
from . import mqtt_wow

def base(request):
    return render(request, "prise/base.html")


def controle_prises(request):
    etat_prise = mqtt_wow.led_state
    return render(request, "prises/controles.html", {"etat_prise": etat_prise})

@csrf_exempt
def switch_led(request):
    print("Reçu /switch_led/", request.method, request.POST)
    if request.method == "POST":
        new_state = request.POST.get("state")
        print("Etat demandé :", new_state)
        if new_state in ["0", "1"]:
            mqtt_wow.client.publish("led/set", new_state)
            return JsonResponse({"success": True, "new_state": new_state})
    return JsonResponse({"success": False}, status=400)