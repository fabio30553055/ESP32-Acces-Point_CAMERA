% ESP32 CAM APLICACIÓN MATLAB 2025
% AUTORES: SANDRA NOPE FABIO RIVADENEIRA
% TALLER DE INGENIERIA 3 2025_2
% UNIVERSIDAD DEL VALLE CALI-COLOMBIA

clear
close all
clc
% URL DE SERVICIOS
IMAGEN_URL   = 'http://192.168.10.1/IMAGEN_RX.jpg';
PIN4_ON_URL = 'http://192.168.10.1/set?pin4=1';
PIN4_OFF_URL = 'http://192.168.10.1/set?pin4=0';
ETIQUETA_URL  = 'http://192.168.10.1/last_label';  

FIGURA_CAM = figure('Name','ESP32-CAM Live + Control','NumberTitle','off',...
              'MenuBar','none','ToolBar','none','Position',[100 100 760 520]);

H_AX  = axes('Parent',FIGURA_CAM,'Units','pixels','Position',[20 80 640 420]);
H_IMG  = imshow(zeros(480,640,3,'uint8'),'Parent',H_AX);

title(H_AX,'ESP32-CAM IMAGEN RX.jpg');

uicontrol('Parent',FIGURA_CAM,'Style','pushbutton','String','PIN ON',...
    'FontSize',11,'Position',[680 420 70 30],...
    'Callback',@(src,evt) webread(PIN4_ON_URL,'state','1'));

uicontrol('Parent',FIGURA_CAM,'Style','pushbutton','String','PIN OFF',...
    'FontSize',11,'Position',[680 380 70 30],...
    'Callback',@(src,evt) webread(PIN4_OFF_URL,'state','0'));

% RECEPCION ETIQUETA
LABEL_ANTERIOR = "";
LECTURA_TEXTO = weboptions('Timeout', 2, 'ContentType','text');%Especifique los parámetros para el servicio web  

tic;   % reloj para consulta de etiqueta
CNT=1;

while ishandle(FIGURA_CAM)
    % Actualizar imagen
    try
        img = imread(IMAGEN_URL);
        set(H_IMG,'CData',img);
        drawnow;
        NEW_LABEL = strtrim(webread(ETIQUETA_URL, LECTURA_TEXTO));
        fprintf('Etiqueta recibida: %s, %d\n', char(NEW_LABEL),CNT);
        CNT=CNT+1;
    catch
        pause(0.02);
    end

    % Leer etiqueta cada 0.5s
    if toc > 0.5
        try
            NEW_LABEL = strtrim(webread(ETIQUETA_URL, LECTURA_TEXTO));
            %NEW_LABEL = lower(string(NEW_LABEL));
        catch
            
        end
        tic; % reiniciar intervalo
    end

    pause(0.1);
end


