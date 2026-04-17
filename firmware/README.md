# Transputer-Teensy-Link-Firmware

Habe nen Augenblicke gebraucht die Meldung  mit SERIAL_9BIT_SUPPORT richt zu lesen 🙂
USB muss auf Serial und Midi stehen... Midi will vielleicht nicht jeder haben
Du hast  #define MIT_DISP128_32 1 benutzt... ich habe 128_64, die etwas größeren der SS106-Display

export TRANSPUTER=$HOME/bin/transputer/link300.so.1.0.1@192.168.239.155

Deinen Test müsstest Du das  #define MIT_LINK_C011 (in der ino-datei)auskommentieren, damit die Tools mit dem teensylink sprechen"

[https://bin.transputer.net](https://bin.transputer.net)



MIT_LINK_C011 und MIT_LINK_TEENSY aktiv
    alle IP-Anfragen gehen auf den C011, ein Aufruf von z.b. rspy, oder iserver -sb xxx.btl triggert einen Reset
    der teensy-Link hängt am USB-Host und transferiert die Device-daten, steuert selbst aber keinen Reset
nur MIT_LINK_C011
    wie 1., aber der USB-Host  benutzt auch den C11. Da muss der Nutzer wissen, was er macht
nur MIT_LINK_TEENSY
    wie 2., nur mit dem Teensy-Link