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
#include "msgs.h"


const char *TEXT_WELCOME = {
    MSGS_NL
    "                                                                 Bienvenido a..." MSGS_NL
    " █████╗  ██████╗███████╗      █████╗ ██╗      █████╗ ██████╗ ███╗   ███╗ █████╗ "
    "██╔══██╗██╔════╝██╔════╝██╗  ██╔══██╗██║     ██╔══██╗██╔══██╗████╗ ████║██╔══██╗"
    "███████║██║     ███████╗╚═╝  ███████║██║     ███████║██████╔╝██╔████╔██║███████║"
    "██╔══██║██║     ╚════██║██╗  ██╔══██║██║     ██╔══██║██╔══██╗██║╚██╔╝██║██╔══██║"
    "██║  ██║╚██████╗███████║╚═╝  ██║  ██║███████╗██║  ██║██║  ██║██║ ╚═╝ ██║██║  ██║"
    "╚═╝  ╚═╝ ╚═════╝╚══════╝     ╚═╝  ╚═╝╚══════╝╚═╝  ╚═╝╚═╝  ╚═╝╚═╝     ╚═╝╚═╝  ╚═╝"
    "Comandos:" MSGS_NL
    MSGS_NL
    "   'i' Este mensaje." MSGS_NL
    "   'p' Ingresar contraseña." MSGS_NL
    "   'x' Cancelar operación." MSGS_NL
    "   'a' Sensores." MSGS_NL
    "   's' Buffers de I/O." MSGS_NL
    "   'm' Máquina de estado." MSGS_NL
    "   'c' Borrar pantalla." MSGS_NL
    MSGS_NL
};

const char *TEXT_WRONGCOMMANDSIZE = {
    MSGS_ERROR("El tamaño de comando es incorrecto.")
};

const char *TEXT_PASSWORDINPUT = {
    MSGS_INFO("Ingrese contraseña y presione [ENTER] para continuar.")
              MSGS_PREFIX_GROUP ": "
};

const char *TEXT_PASSWORDTOARM = {
    MSGS_INFO("Alarma " TERM_COLOR_BOLD_GREEN "DESARMADA" TERM_NO_COLOR
              ", presione 'p' para ARMAR.")
};

const char *TEXT_INVALIDPASSWORD = {
    MSGS_WARNING("Contraseña inválida.")
};

const char *TEXT_CHECKINGPASSWORD = {
    MSGS_INFO("Comparando contraseña...")
};

const char *TEXT_WRONGPASSWORD = {
    MSGS_WARNING("Contraseña incorrecta.")
};

const char *TEXT_PASSWORDMATCH = {
    MSGS_INFO("Contraseña OK.")
};

const char *TEXT_ALARMARMINGBEGIN = {
    MSGS_INFO("En %1 segundo(s) la alarma cambiará a estado ARMADA.")
};

const char *TEXT_ALARMARMINGCANCELLED = {
    MSGS_WARNING("Armado de alarma CANCELADO.")
};

const char *TEXT_ALARMARMINGEND = {
    MSGS_INFO("Armando alarma...")
};

const char *TEXT_ARMEDPASSWORDTODISARM = {
    MSGS_INFO("Alarma " TERM_COLOR_BOLD_RED "ARMADA" TERM_NO_COLOR
              ", presione 'p' para DESARMAR.")
};

const char *TEXT_SENSORWINDOW = {
    MSGS_WARNING("SENSOR DE VENTANA ACTIVADO.")
};

const char *TEXT_SENSORDOOR = {
    MSGS_WARNING("SENSOR DE PUERTA ACTIVADO.")
};

const char *TEXT_INTRUDERPASSWORDTODISARM = {
    MSGS_CRITICAL("ALERTA DE INTRUSO! Presione 'p' para CANCELAR.")
};

const char *TEXT_DOORPASSWORDTODISARM = {
    MSGS_INFO("Modo " TERM_COLOR_BOLD_RED "INTRUSO" TERM_NO_COLOR
              " se activará en %1 segundo(s). Presione 'p' para CANCELAR.")
};

const char *TEXT_DOORPASSWORDTODISARMAGAIN = {
    MSGS_WARNING("%1 intento(s) disponible(s).")
};

const char *TEXT_WRONGCOMMAND = {
    MSGS_ERROR("Comando no reconocido.")
};

const char *TEXT_INVALIDTASKSTATUS = {
    MSGS_ERROR("Tarea en estado invalido.")
};
