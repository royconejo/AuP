/*
    CESE Actividad 5: ALARMA
    Copyright 2018 Santiago Germino (royconejo@gmail.com)

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    1.  Redistributions of source code must retain the above copyright notice,
        this list of conditions and the following disclaimer.

    2.  Redistributions in binary form must reproduce the above copyright
        notice, this list of conditions and the following disclaimer in the
        documentation and/or other materials provided with the distribution.

    3.  Neither the name of the copyright holder nor the names of its
        contributors may be used to endorse or promote products derived from
        this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
    AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
    IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
    ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
    LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
    CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
    SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
    INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
    CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
    ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
    POSSIBILITY OF SUCH DAMAGE.
*/
#include "text_app.h"
#include "texstyle.h"


const char *TEXT_WELCOME = {
    TEXSTYLE_NL
    "                                                                 Bienvenido a..." TEXSTYLE_NL
    " █████╗  ██████╗███████╗      █████╗ ██╗      █████╗ ██████╗ ███╗   ███╗ █████╗ "
    "██╔══██╗██╔════╝██╔════╝██╗  ██╔══██╗██║     ██╔══██╗██╔══██╗████╗ ████║██╔══██╗"
    "███████║██║     ███████╗╚═╝  ███████║██║     ███████║██████╔╝██╔████╔██║███████║"
    "██╔══██║██║     ╚════██║██╗  ██╔══██║██║     ██╔══██║██╔══██╗██║╚██╔╝██║██╔══██║"
    "██║  ██║╚██████╗███████║╚═╝  ██║  ██║███████╗██║  ██║██║  ██║██║ ╚═╝ ██║██║  ██║"
    "╚═╝  ╚═╝ ╚═════╝╚══════╝     ╚═╝  ╚═╝╚══════╝╚═╝  ╚═╝╚═╝  ╚═╝╚═╝     ╚═╝╚═╝  ╚═╝"
    "Comandos:" TEXSTYLE_NL
    TEXSTYLE_NL
    "   'i' Este mensaje." TEXSTYLE_NL
    "   'p' Ingresar contraseña." TEXSTYLE_NL
    "   'x' Cancelar operación." TEXSTYLE_NL
    "   'a' Sensores." TEXSTYLE_NL
    "   's' Buffers de I/O." TEXSTYLE_NL
    "   'm' Máquina de estado." TEXSTYLE_NL
    "   'c' Borrar pantalla." TEXSTYLE_NL
    TEXSTYLE_NL
};

const char *TEXT_WRONGCOMMANDSIZE = {
    TEXSTYLE_ERROR("El tamaño de comando es incorrecto.")
};

const char *TEXT_PASSWORDINPUT = {
    TEXSTYLE_INFO("Ingrese contraseña y presione [ENTER] para continuar.")
              TEXSTYLE_PREFIX_GROUP ": "
};

const char *TEXT_PASSWORDTOARM = {
    TEXSTYLE_INFO("Alarma " TERM_FG_COLOR_BOLD_GREEN "DESARMADA" TERM_NO_COLOR
              ", presione 'p' para ARMAR.")
};

const char *TEXT_INVALIDPASSWORD = {
    TEXSTYLE_WARNING("Contraseña inválida.")
};

const char *TEXT_CHECKINGPASSWORD = {
    TEXSTYLE_INFO("Comparando contraseña...")
};

const char *TEXT_WRONGPASSWORD = {
    TEXSTYLE_WARNING("Contraseña incorrecta.")
};

const char *TEXT_PASSWORDMATCH = {
    TEXSTYLE_INFO("Contraseña OK.")
};

const char *TEXT_ALARMARMINGBEGIN = {
    TEXSTYLE_INFO("En %1 segundo(s) la alarma cambiará a estado ARMADA. "
                  "Cancelar con 'x'.")
};

const char *TEXT_ALARMARMINGCANCELLED = {
    TEXSTYLE_WARNING("Armado de alarma CANCELADO.")
};

const char *TEXT_ALARMARMINGEND = {
    TEXSTYLE_INFO("Armando alarma...")
};

const char *TEXT_ARMEDPASSWORDTODISARM = {
    TEXSTYLE_INFO("Alarma " TERM_FG_COLOR_BOLD_RED "ARMADA" TERM_NO_COLOR
              ", presione 'p' para DESARMAR.")
};

const char *TEXT_SENSORWINDOW = {
    TEXSTYLE_WARNING("SENSOR DE VENTANA ACTIVADO.")
};

const char *TEXT_SENSORDOOR = {
    TEXSTYLE_WARNING("SENSOR DE PUERTA ACTIVADO.")
};

const char *TEXT_INTRUDERPASSWORDTODISARM = {
    TEXSTYLE_CRITICAL("ALERTA DE INTRUSO! Presione 'p' para CANCELAR.")
};

const char *TEXT_DOORPASSWORDTODISARM = {
    TEXSTYLE_INFO("Modo " TERM_FG_COLOR_BOLD_RED "INTRUSO" TERM_NO_COLOR
              " se activará en %1 segundo(s). Presione 'p' para CANCELAR.")
};

const char *TEXT_DOORPASSWORDTODISARMAGAIN = {
    TEXSTYLE_WARNING("%1 intento(s) disponible(s).")
};

const char *TEXT_WRONGCOMMAND = {
    TEXSTYLE_ERROR("Comando no reconocido.")
};

const char *TEXT_INVALIDTASKSTATUS = {
    TEXSTYLE_ERROR("Tarea en estado invalido.")
};
