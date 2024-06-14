var gateway = `ws://${window.location.hostname}/ws`;
var websocket;
window.addEventListener('load', onLoad);
function onLoad(event) {
    initWebSocket();
    initJoystick();
}
function initWebSocket() {
    console.log('Trying to open a WebSocket connection...');
    websocket = new WebSocket(gateway);
    websocket.onopen    = onOpen;
    websocket.onclose   = onClose;
    websocket.onmessage = onMessage;
}
function onOpen(event) {
    console.log('Connection opened');
    PopUpMessage();
}
function onClose(event) {
    console.log('Connection closed');
    setTimeout(initWebSocket, 2000);
}

function PopUpMessage(message, delay = 2000){
    let div = document.createElement('div');
    div.id = 'popUpMessage';
    let me = document.createElement('p');
    me.textContent = message;
    div.append(me);
    document.body.append(div);
    setTimeout(function(){
        div.remove();
    }, delay);
}

function onMessage(event) {}
var ctx = null;
var canvas = null;
stickX = 0;
stickY = 0;
joystickMargin = 100;
mouseOnScreen = false;
joystickRadius = 100;
joystickMaxForse = 1024;
packetSendms = 50;
timer = null;
function initJoystick(){
    canvas = document.getElementById("canvas_joystick");
    ctx = canvas.getContext('2d');
    canvas.addEventListener('mouseup', onMouseUp);
    canvas.addEventListener('mousedown', onMouseDown);
    canvas.addEventListener('mousemove', onMouseMove);
    canvas.addEventListener('mouseleave', onMouseUp);
    canvas.width = window.innerWidth;
    canvas.height = window.innerHeight;
    console.log(canvas.width, canvas.height);
    joystickRadius = Math.min(canvas.width / 2 - joystickMargin, canvas.height / 2 - joystickMargin);
    console.log(joystickRadius);
    timer = Date.now();
    DrawStick();
}
function DrawStick(){
    ctx.reset();
    ctx.fillRect(0, 0, canvas.width, canvas.height);
    ctx.beginPath();
    ctx.arc(canvas.width / 2, canvas.height / 2, joystickRadius, 0, 2 * Math.PI, false);
    ctx.fillStyle = 'white';
    ctx.fill();
    ctx.beginPath();
    ctx.arc(stickX + canvas.width / 2, -(stickY) + canvas.height / 2, joystickRadius / 10, 0, 2 * Math.PI, false);
    ctx.fillStyle = 'red';
    ctx.fill();
}
function onMouseUp(event){
    mouseOnScreen = false;
    stickX = 0;
    stickY = 0;
    DrawStick();
    send({'type' : 1, 'dirX': 0, 'dirY' : 0, 'power' : 0}, true);
}   

function send(obj, delay = true){
    var pack = JSON.stringify(obj, 
    function(key, value){
        return typeof value == 'number' ? Number(value.toFixed(2)) : value;
    });
        
        if(delay || (Date.now() - timer) > packetSendms){
            try{
                websocket.send(pack);
            }catch(e){
                console.log(e);
                PopUpMessage("Ошибка отправки сообщения. Подождите.", 1000);
            }
            console.log(pack);
            timer = Date.now();
        }
}

function onMouseMove(event){
    let x = event.x - canvas.width / 2;
    let y = -(event.y - canvas.height / 2);
    let length = Math.sqrt(x * x + y * y);
    if(mouseOnScreen && length <= joystickRadius){
        stickX = x;
        stickY = y;
        DrawStick();
        send({'type' : 1, 'dirX': stickX / length, 'dirY' : stickY / length, 'power' : Math.trunc(length * joystickMaxForse / joystickRadius)}, true);
    }
}        
function onMouseDown(event){
    mouseOnScreen = true;
    console.log(event.x, event.y);
}