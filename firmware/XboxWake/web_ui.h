#pragma once
#include <Arduino.h>

// Entire UI is served from flash. No CDN, build step, or filesystem is required.
const char WEB_UI[] PROGMEM = R"HTML(<!doctype html>
<html lang="ru">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<meta name="color-scheme" content="light">
<meta name="theme-color" content="#f3f4ef">
<title>Xbox Wake — управление</title>
<style>
:root{--bg:#f3f4ef;--paper:#fff;--ink:#20261f;--muted:#697066;--line:#dfe3da;--green:#206c36;--soft:#e5f3e6;--red:#a42f2e;--amber:#86620b;font:15px/1.5 system-ui,-apple-system,"Segoe UI",sans-serif;color:var(--ink);background:var(--bg)}
*{box-sizing:border-box}body{margin:0}button,input,select{font:inherit}button,a,input,select{touch-action:manipulation}button{cursor:pointer}button:disabled{cursor:default;opacity:.48}button,input,select{border-radius:9px}button:focus-visible,input:focus-visible,select:focus-visible,summary:focus-visible{outline:3px solid #86ad78;outline-offset:3px}input:not([type=checkbox]),select{width:100%;padding:10px 12px;border:1px solid #ccd2c5;background:#fff;color:var(--ink);min-width:0}input::placeholder{color:#858b81}h1,h2,h3,p{margin:0}h1{font-size:26px;letter-spacing:-1px;line-height:1.2}h2{font-size:18px;letter-spacing:-.4px}h3{font-size:15px}small,.hint{font-size:12px;line-height:1.5;color:var(--muted)}label.field{display:grid;gap:6px;font-size:13px;font-weight:600}label.field .hint{font-weight:400}.shell{max-width:1180px;margin:auto;padding:34px 28px 26px}.top{display:flex;align-items:center;justify-content:space-between;gap:20px;margin-bottom:27px}.brand{display:flex;align-items:center;gap:13px}.logo{display:grid;place-items:center;background:var(--ink);color:white;width:44px;height:44px;border-radius:13px}.brand p{font-size:12px;color:var(--muted);margin-top:5px}.eyebrow{color:var(--muted);font-size:11px;letter-spacing:1.3px;text-transform:uppercase;font-weight:700}.pill{display:inline-flex;align-items:center;gap:7px;font-size:12px;padding:6px 11px;border-radius:30px;background:#e9ece4;color:#525c4b}.dot{width:7px;height:7px;background:currentColor;border-radius:50%}.pill.ok{background:var(--soft);color:var(--green)}.pill.warn{background:#fbefcf;color:var(--amber)}.pill.error{background:#f8e3df;color:var(--red)}.master{display:flex;align-items:center;justify-content:space-between;gap:25px;border:1px solid #cadac5;background:#e7eddf;padding:25px 27px;border-radius:17px}.master h2{font-size:23px;letter-spacing:-.6px;margin:6px 0}.master p{font-size:13px;color:#53604e;max-width:720px}.master.off{background:#eceee8;border-color:var(--line)}.master-label{display:flex;align-items:center;gap:12px;flex:none;font-size:13px;font-weight:600}.switch{position:relative;display:inline-flex;width:43px;height:25px;flex:none;vertical-align:middle}.switch input{position:absolute;opacity:0;width:100%;height:100%;margin:0;z-index:2;cursor:pointer}.switch span{width:100%;height:100%;border-radius:25px;background:#aeb6a7;transition:background .16s}.switch span:before{content:"";position:absolute;width:19px;height:19px;left:3px;top:3px;border-radius:50%;background:white;box-shadow:0 1px 3px #0003;transition:transform .16s}.switch input:checked+span{background:var(--green)}.switch input:checked+span:before{transform:translateX(18px)}.switch input:focus-visible+span{outline:3px solid #86ad78;outline-offset:3px}.switch input:disabled{cursor:default}.switch input:disabled+span{opacity:.5}.switch.large{width:62px;height:35px}.switch.large span:before{width:27px;height:27px;left:4px;top:4px}.switch.large input:checked+span:before{transform:translateX(27px)}.layout{display:grid;grid-template-columns:minmax(0,1.13fr) minmax(0,1fr);gap:20px;align-items:start;margin-top:24px}.column{display:grid;gap:20px}.card{background:var(--paper);border:1px solid var(--line);border-radius:15px;overflow:hidden}.card-head{display:flex;align-items:center;justify-content:space-between;gap:12px;padding:21px 22px 16px}.card-head .hint{margin-top:5px}.card-body{padding:0 22px 22px}.count{font-size:12px;border-radius:5px;background:#edf0e9;padding:3px 8px;color:#647059}.controllers{border-top:1px solid var(--line)}.controller{padding:17px 22px;border-bottom:1px solid var(--line)}.controller-main{display:flex;align-items:center;gap:12px}.device-icon{width:35px;height:35px;display:grid;place-items:center;border-radius:9px;background:#f0f3ec;color:#52654a;flex:none}.device-info{min-width:0;flex:1}.device-name{border:1px solid transparent!important;padding:2px 4px!important;margin-left:-4px;border-radius:5px!important;font-size:14px!important;font-weight:650;background:transparent!important;max-width:280px}.device-name:hover,.device-name:focus{border-color:#ccd2c5!important;background:#fff!important}.mono{font-family:ui-monospace,SFMono-Regular,Consolas,monospace;letter-spacing:.15px}.device-meta{display:flex;align-items:center;flex-wrap:wrap;gap:5px 10px;font-size:11px;color:var(--muted);margin-top:4px}.device-bottom{display:flex;gap:10px;align-items:center;justify-content:space-between;margin-top:11px;font-size:11px;color:var(--muted)}.row-actions{display:flex;gap:10px}.text-button{border:0;background:transparent;padding:2px 0;color:var(--green);font-size:12px}.text-button.danger{color:var(--red)}.btn{border:1px solid #ccd2c5;background:#fff;padding:9px 13px;font-size:13px;font-weight:600;color:var(--ink);line-height:1.3;min-height:38px}.btn:hover:enabled{background:#f4f6f0}.btn.primary{background:var(--green);border-color:var(--green);color:white}.btn.primary:hover:enabled{background:#175c2b}.btn.soft{background:#eff4ea;border-color:#e0e8d8;color:#3b5f2d}.btn.small{padding:7px 10px;font-size:12px;min-height:32px}.section-space{padding:19px 22px 22px}.section-space h3{margin-bottom:12px}.form-grid{display:grid;grid-template-columns:1fr 1fr;gap:14px}.form-stack{display:grid;gap:14px}.full{grid-column:1/-1}.actions{display:flex;align-items:center;justify-content:space-between;gap:10px;flex-wrap:wrap;margin-top:17px}.inline{display:flex;align-items:center;gap:9px}.check{display:flex;align-items:center;gap:7px;font-size:12px;color:var(--muted);cursor:pointer}.check input{accent-color:var(--green);width:15px;height:15px}.divider{height:1px;background:var(--line);margin:20px 0}.scan-area{border-top:1px solid var(--line);background:#fafbf8;padding:18px 22px}.scan-heading{display:flex;align-items:center;justify-content:space-between;gap:10px;margin-bottom:10px}.scan-list{display:grid;gap:8px;margin-top:14px;max-height:325px;overflow:auto}.scan-device{display:flex;align-items:center;gap:10px;padding:10px;background:white;border:1px solid var(--line);border-radius:9px}.scan-info{flex:1;min-width:0}.scan-name{font-size:12px;font-weight:600;overflow-wrap:anywhere}.scan-detail{font-size:10px;color:var(--muted);overflow-wrap:anywhere}.tag{display:inline-block;border-radius:4px;padding:1px 5px;font-size:10px;background:#e8f0e2;color:#526747;margin:4px 0}.tag.neutral{background:#edf0e9;color:#697066}.empty{padding:24px 20px;text-align:center;font-size:13px;color:var(--muted)}details{border-top:1px solid var(--line)}summary{cursor:pointer;padding:17px 22px;font-size:13px;font-weight:600}details .card-body{padding-top:0}.details-fields{padding-top:1px}.note{padding:10px 12px;background:#f5f7f1;border-radius:8px;font-size:12px;line-height:1.6;color:#64705b}.events{padding:0 22px 19px;max-height:265px;overflow:auto}.event{display:flex;gap:12px;font-size:12px;padding:9px 0;border-top:1px solid #edf0e9}.event time{color:var(--muted);min-width:68px;flex:none;font-variant-numeric:tabular-nums}.event p{overflow-wrap:anywhere}.feedback{display:flex;align-items:center;justify-content:space-between;gap:12px;border:1px solid #b9d6b6;background:#eff8ea;color:#34542b;padding:12px 16px;margin-bottom:16px;border-radius:10px;font-size:13px}.feedback.error{border-color:#ecc2bd;background:#fff0ec;color:var(--red)}.feedback button{border:0;color:inherit;background:transparent;font-size:22px;line-height:1}.footer{display:flex;justify-content:space-between;gap:12px;margin-top:24px;font-size:11px;color:#7a8272}.loading .master{opacity:.6}[hidden]{display:none!important}.noscript{padding:25px;background:#fff0ec;color:var(--red)}
@media(max-width:850px){.layout{grid-template-columns:1fr}.shell{max-width:650px;padding:24px 18px}.master{padding:21px}.master h2{font-size:21px}.master-label{flex-direction:column-reverse;gap:5px}.top{margin-bottom:22px}}
@media(max-width:420px){.shell{padding:20px 12px}.brand{gap:9px}h1{font-size:23px}.brand p{font-size:10px}.logo{width:37px;height:37px}.top{gap:8px}.pill{padding:5px 8px;font-size:10px}.master{gap:12px;padding:18px}.master h2{font-size:19px}.master p{font-size:12px}.master .eyebrow{font-size:10px}.card-head,.controller,.section-space{padding-left:17px;padding-right:17px}.card-body,.scan-area,.events{padding-left:17px;padding-right:17px}.form-grid{gap:12px}.form-grid.mobile-stack{grid-template-columns:1fr}.footer{flex-direction:column;gap:3px}.device-bottom{align-items:start}.row-actions{flex:none}}
</style>
</head>
<body class="loading">
<noscript><div class="noscript">Для управления ESP32 включите JavaScript в браузере.</div></noscript>
<main class="shell">
  <header class="top">
    <div class="brand"><div class="logo" aria-hidden="true"><svg width="27" height="27" viewBox="0 0 28 28" fill="none"><path d="M7 9h14c2 0 3 2 3.5 5l1 5c.5 3-2.5 4-4 2l-3-3h-9l-3 3c-1.5 2-4.5 1-4-2l1-5C4 11 5 9 7 9Z" stroke="currentColor" stroke-width="1.7"/><path d="M9 12v5m-2.5-2.5h5" stroke="currentColor" stroke-width="1.7" stroke-linecap="round"/><circle cx="19" cy="13" r="1" fill="currentColor"/><circle cx="22" cy="16" r="1" fill="currentColor"/><path d="M13 4v4m-2-2 2 2 2-2" stroke="currentColor" stroke-width="1.4" stroke-linecap="round" stroke-linejoin="round"/></svg></div><div><h1>Xbox Wake</h1><p>Кнопка Xbox — компьютер проснулся.</p></div></div>
  </header>
  <div id="feedback" class="feedback" role="status" hidden><span id="feedbackText"></span><button type="button" id="dismissFeedback" aria-label="Закрыть уведомление">×</button></div>
  <section class="master" id="masterCard" aria-labelledby="masterHeading">
    <div><div class="eyebrow">Пробуждение по Bluetooth</div><h2 id="masterHeading">Получаем состояние…</h2><p id="masterDescription">ESP32 распознаёт включение зарегистрированного контроллера и отправляет Wake-on-LAN компьютеру.</p></div>
    <label class="master-label"><span id="masterLabel">Главный</span><span class="switch large"><input id="masterEnabled" type="checkbox" role="switch" aria-label="Разрешить пробуждение от всех активных контроллеров" disabled><span></span></span></label>
  </section>
  <div class="layout">
    <div class="column">
      <section class="card" aria-labelledby="controllersHeading"><div class="card-head"><div><h2 id="controllersHeading">Контроллеры</h2></div><span class="count" id="controllerCount">0</span></div>
        <div class="controllers" id="controllers"><div class="empty" id="controllersEmpty">Загружаем контроллеры…</div></div>
        <div class="section-space"><h3>Добавить контроллер</h3><form id="controllerForm" class="form-stack"><div class="form-grid mobile-stack"><label class="field">Название<input id="newLabel" name="label" maxlength="40" placeholder="Например, белый Xbox" required></label><label class="field">Bluetooth MAC<input id="newMac" name="mac" class="mono" maxlength="17" placeholder="14:CB:65:8E:09:FB" autocomplete="off" autocapitalize="characters" spellcheck="false" required></label></div><label class="field">Тип Bluetooth-адреса<select id="newAddressType" name="addressType"><option value="any">Любой — если тип неизвестен</option><option value="public">Public — публичный</option><option value="random">Random — случайный</option></select><span class="hint">При выборе из списка рядом тип заполняется автоматически. Меняющийся случайный MAC не подходит для постоянной регистрации.</span></label><div class="actions" style="margin-top:0"><button class="btn primary" type="submit" id="addButton" disabled>Добавить</button><button class="btn soft" type="button" id="scanButton" disabled>Найти рядом</button></div></form></div>
        <div class="scan-area" id="scanArea" hidden><div class="scan-heading"><h3>Устройства рядом</h3><button class="text-button" type="button" id="closeScan">Скрыть</button></div><p class="hint">Выключите геймпад и нажмите Xbox. Список обновляется автоматически. Кнопку сопряжения нажимать не нужно.</p><label class="check" style="margin-top:12px"><input type="checkbox" id="showAll">Показать все BLE-устройства</label><div class="scan-list" id="scanList"><div class="empty">Сканируем эфир…</div></div><p class="hint" style="margin-top:12px">Метки Xbox и Microsoft — только подсказки. Проверьте MAC своего контроллера перед добавлением.</p></div>
      </section>
      <section class="card" aria-labelledby="eventsHeading"><div class="card-head"><div><h2 id="eventsHeading">Журнал событий</h2></div></div><div class="events" id="events"><div class="empty">Пока нет событий</div></div></section>
    </div>
    <div class="column">
      <section class="card" aria-labelledby="targetHeading"><div class="card-head"><div><h2 id="targetHeading">Компьютер</h2><p class="hint">Куда отправлять команду пробуждения.</p></div></div><form id="targetForm"><div class="card-body form-stack"><div class="form-grid"><label class="field">IP компьютера<input id="targetIp" name="ip" class="mono" placeholder="192.168.1.100" inputmode="decimal" autocomplete="off" spellcheck="false" required></label><label class="field">UDP-порт<input id="targetPort" name="port" type="number" min="1" max="65535" value="9" required></label></div><div class="actions" style="margin-top:0"><button class="btn soft" type="button" id="resolveMac" disabled>Определить MAC по IP</button></div><p class="hint" id="resolveHint" role="status">Включите компьютер и укажите его IP. При сохранении пустое поле MAC заполнится автоматически через ARP. Найденный адрес сохранится для пробуждения выключенного ПК.</p><label class="field">MAC сетевой карты ПК<input id="targetMac" name="mac" class="mono" maxlength="17" placeholder="Определится по IP — или введите вручную" autocomplete="off" autocapitalize="characters" spellcheck="false"></label><label class="field">Способ доставки<select id="targetMode" name="mode"><option value="broadcast">Broadcast — всей локальной сети</option><option value="ethernet">Ethernet broadcast — без UDP и ARP</option></select></label><div class="actions" style="margin-top:0"><button class="btn primary" type="submit" id="saveTarget" disabled>Сохранить</button></div></div>
          <details><summary>Доставка и защита от повторов</summary><div class="card-body details-fields form-stack"><label class="field">Broadcast-адрес<input id="targetBroadcast" name="broadcast" class="mono" value="auto" placeholder="auto или 192.168.1.255" autocomplete="off" spellcheck="false"><span class="hint">auto — вычислить по IP и маске сети ESP32. Поле используется только для broadcast.</span></label><div class="form-grid"><label class="field">Тишина в эфире, с<input id="quietSeconds" name="quietSeconds" type="number" min="3" max="300" value="15" required></label><label class="field">Пауза между WoL, с<input id="cooldownSeconds" name="cooldownSeconds" type="number" min="3" max="3600" value="30" required></label></div><p class="hint">Повторное появление того же контроллера учитывается после периода без его BLE-объявлений. Общая пауза ограничивает частоту отправки WoL. Это не проверка состояния ПК.</p><button class="btn" type="submit">Сохранить все настройки ПК</button></div></details>
        </form>
      </section>
      <section class="card" aria-labelledby="networkHeading"><div class="card-head"><div><h2 id="networkHeading">Подключение к Wi-Fi</h2><p class="hint">ESP32 и компьютер должны быть в одной локальной сети.</p></div></div><div class="card-body"><form id="wifiForm" class="form-stack"><label class="field">Имя сети, SSID<input id="wifiSsid" name="ssid" maxlength="32" autocomplete="off" required placeholder="Домашняя сеть 2,4 ГГц"></label><label class="field">Пароль<input id="wifiPassword" name="password" type="password" maxlength="63" autocomplete="new-password" placeholder="Введите пароль выбранной сети"><span class="hint">При каждом сохранении введите пароль заново. Пустое поле удалит прежний пароль — это подходит только для открытой сети.</span></label><div class="actions" style="margin-top:0"><button class="btn" type="submit" id="saveWifi" disabled>Сохранить и подключиться</button></div></form><div class="divider"></div><p class="hint" id="networkInfo">Информация о подключении появится после загрузки.</p><p class="hint" id="apInfo"></p></div></section>
    </div>
  </div>
  <footer class="footer"><span id="uptime">Панель работает локально, без облака</span></footer>
</main>
<template id="controllerTemplate"><article class="controller"><div class="controller-main"><div class="device-icon" aria-hidden="true"><svg width="23" height="23" viewBox="0 0 28 28" fill="none"><path d="M7 8h14c2 0 3 2 3.5 5l1 5c.5 3-2.5 4-4 2l-3-3h-9l-3 3c-1.5 2-4.5 1-4-2l1-5C4 10 5 8 7 8Z" stroke="currentColor" stroke-width="1.7"/><path d="M9 11v5m-2.5-2.5h5" stroke="currentColor" stroke-width="1.7" stroke-linecap="round"/><circle cx="19" cy="12" r="1" fill="currentColor"/><circle cx="22" cy="15" r="1" fill="currentColor"/></svg></div><div class="device-info"><input class="device-name" maxlength="40" aria-label="Название контроллера"><div class="device-meta"><span class="mono device-mac"></span><span class="device-signal"></span></div></div><label class="switch"><input type="checkbox" class="device-enabled" role="switch" aria-label="Контроллер может будить ПК"><span></span></label></div><div class="device-bottom"><div class="row-actions"><button class="text-button danger device-delete" type="button">Удалить</button></div></div></article></template>
<template id="scanTemplate"><article class="scan-device"><div class="scan-info"><div class="scan-name"></div><span class="tag"></span><div class="scan-detail mono"></div></div><button class="btn small scan-add" type="button">Добавить</button></article></template>
<script>
'use strict';
const $ = id => document.getElementById(id);
const model = {state:null, token:'', stateTask:null, scanBusy:false, scans:[], scanRows:new Map(), scanEmpty:null, loaded:false, pending:new Set(), rows:new Map(), targetDirty:false};
const macPattern = /^[0-9A-F]{2}(?::[0-9A-F]{2}){5}$/;
const normalizeMac = value => value.trim().replace(/-/g,':').toUpperCase();
const ipv4 = value => {const p=value.trim().split('.');return p.length===4&&p.every(x=>/^\d{1,3}$/.test(x)&&Number(x)<=255);};
const age = seconds => seconds==null?'не обнаружен':seconds<2?'только что':seconds<60?Math.floor(seconds)+' с назад':seconds<3600?Math.floor(seconds/60)+' мин назад':Math.floor(seconds/3600)+' ч назад';
function feedback(message,error=false){$('feedbackText').textContent=message;$('feedback').className='feedback'+(error?' error':'');$('feedback').setAttribute('role',error?'alert':'status');$('feedback').hidden=false;}
$('dismissFeedback').addEventListener('click',()=>{$('feedback').hidden=true;});
async function api(path,data){
  const controller=new AbortController();const timer=setTimeout(()=>controller.abort(),9000);
  try{const options={signal:controller.signal,cache:'no-store'};if(data!==undefined){options.method='POST';options.headers={'Content-Type':'application/json','X-Wake-Token':model.token};options.body=JSON.stringify(data);}
    const response=await fetch('/api/'+path,options);let body;try{body=await response.json();}catch(_){throw new Error('ESP32 вернула непонятный ответ. Обновите страницу.');}
    if(!response.ok||body.error)throw new Error(body.error||'Ошибка запроса: '+response.status);return body;
  }catch(error){if(error.name==='AbortError')throw new Error('ESP32 не ответила за 9 секунд. Проверьте подключение.');if(error instanceof TypeError)throw new Error('Нет связи с ESP32. Проверьте Wi-Fi и адрес панели.');throw error;}finally{clearTimeout(timer);}
}
async function refresh(force=false){
  if(model.stateTask){await model.stateTask;if(!force)return;}
  const task=(async()=>{try{const state=await api('state');model.state=state;model.token=state.token;renderState(state);if(!model.loaded){fillForms(state);model.loaded=true;document.body.classList.remove('loading');}}
    catch(error){if(!model.loaded){$('masterHeading').textContent='Не удалось подключиться';$('masterDescription').textContent=error.message;}}})();
  model.stateTask=task;try{await task;}finally{if(model.stateTask===task)model.stateTask=null;}
}
async function write(key,path,payload,success){
  if(!model.token){feedback('Дождитесь подключения к ESP32.',true);return false;}
  if(model.pending.has(key))return false;model.pending.add(key);updateBusy();
  try{await api(path,payload);if(success)feedback(success);await refresh(true);return true;}
  catch(error){feedback(error.message,true);return false;}
  finally{model.pending.delete(key);updateBusy();}
}
function updateBusy(){
  const ready=!!model.state;$('masterEnabled').disabled=!ready||model.pending.has('master');$('addButton').disabled=!ready||model.pending.has('add');$('scanButton').disabled=!ready;$('saveTarget').disabled=!ready||model.pending.has('target')||model.pending.has('resolve');$('resolveMac').disabled=!ready||model.pending.has('resolve')||model.pending.has('target');$('resolveMac').textContent=model.pending.has('resolve')?'Ищем MAC…':'Определить MAC по IP';$('targetIp').disabled=model.pending.has('resolve');$('targetMac').disabled=model.pending.has('resolve');$('saveWifi').disabled=!ready||model.pending.has('wifi');
  $('targetForm').querySelectorAll('button[type=submit]').forEach(button=>button.disabled=!ready||model.pending.has('target')||model.pending.has('resolve'));
  for(const [mac,row] of model.rows){const pending=model.pending.has('controller:'+mac);row.enabled.disabled=pending;row.name.disabled=pending;row.remove.disabled=pending;}
}
function renderState(s){
  if(!model.pending.has('master'))$('masterEnabled').checked=s.enabled;
  $('masterCard').classList.toggle('off',!s.enabled);$('masterLabel').textContent=s.enabled?'Включено':'Выключено';
  $('masterHeading').textContent=!s.enabled?'Пробуждение выключено':s.status.ready?'Готов к пробуждению':'Нужна настройка';
  $('masterDescription').textContent=!s.enabled?'Ни один контроллер не отправит команду пробуждения.':s.status.ready?'Включите активный геймпад кнопкой Xbox — ESP32 отправит компьютеру Wake-on-LAN.':(s.status.reason||'Настройте Wi-Fi и сетевую карту компьютера.');
  $('controllerCount').textContent=s.controllers.length;
  $('uptime').textContent='Время работы: '+Math.floor(s.status.uptimeSeconds/3600)+' ч '+Math.floor(s.status.uptimeSeconds%3600/60)+' мин';
  $('networkInfo').textContent=s.wifi.connected?'Адрес в домашней сети: '+s.wifi.ip+'.':'';
  $('apInfo').textContent=s.wifi.apSsid?'Точка настройки: «'+s.wifi.apSsid+'», адрес '+s.wifi.apIp+'.':'';
  renderControllers(s.controllers);renderEvents(s.events||[]);updateBusy();
}
function fillForms(s){
  $('targetIp').value=s.target.ip||'';$('targetMac').value=s.target.mac||'';$('targetMode').value=s.target.mode||'broadcast';$('targetBroadcast').value=s.target.broadcast||'auto';$('targetPort').value=s.target.port||9;$('quietSeconds').value=s.timing.quietSeconds;$('cooldownSeconds').value=s.timing.cooldownSeconds;$('wifiSsid').value=s.wifi.ssid||'';updateTargetFields();
}
function createController(c){
  const el=$('controllerTemplate').content.firstElementChild.cloneNode(true);const row={el,data:c,name:el.querySelector('.device-name'),enabled:el.querySelector('.device-enabled'),remove:el.querySelector('.device-delete'),dirty:false,confirmTimer:null};
  async function saveName(){
    if(!row.dirty)return;
    const label=row.name.value.trim();
    if(!label){row.name.value=row.data.label;row.dirty=false;return;}
    const ok=await write('controller:'+c.mac,'controller',{mac:c.mac,label,enabled:row.data.enabled,addressType:row.data.addressType||'any'},'Название сохранено.');
    if(ok){row.dirty=false;row.name.value=label;}
  }
  row.name.addEventListener('input',()=>{row.dirty=true;});
  row.name.addEventListener('blur',saveName);
  row.name.addEventListener('keydown',event=>{if(event.key==='Enter'){event.preventDefault();row.name.blur();}if(event.key==='Escape'){row.name.value=row.data.label;row.dirty=false;}});
  row.enabled.addEventListener('change',async()=>{const wanted=row.enabled.checked;const ok=await write('controller:'+c.mac,'controller',{mac:c.mac,label:row.data.label,enabled:wanted,addressType:row.data.addressType||'any'},wanted?'Контроллер активирован.':'Контроллер отключён.');if(!ok)row.enabled.checked=row.data.enabled;});
  row.remove.addEventListener('click',async()=>{if(!row.confirmTimer){row.remove.textContent='Точно удалить?';row.confirmTimer=setTimeout(()=>{row.confirmTimer=null;row.remove.textContent='Удалить';},4000);return;}clearTimeout(row.confirmTimer);row.confirmTimer=null;row.remove.textContent='Удалить';await write('controller:'+c.mac,'controller/delete',{mac:c.mac},'Контроллер удалён.');});
  $('controllers').append(el);model.rows.set(c.mac,row);return row;
}
function renderControllers(controllers){
  const existing=new Set(controllers.map(c=>c.mac));for(const [mac,row] of model.rows){if(!existing.has(mac)){clearTimeout(row.confirmTimer);row.el.remove();model.rows.delete(mac);}}
  $('controllersEmpty').hidden=controllers.length>0;$('controllersEmpty').textContent='Добавьте контроллер по MAC или найдите его рядом.';
  for(const c of controllers){const row=model.rows.get(c.mac)||createController(c);row.data=c;if(!row.dirty&&document.activeElement!==row.name)row.name.value=c.label;row.name.setAttribute('aria-label','Название контроллера '+c.mac);if(!model.pending.has('controller:'+c.mac))row.enabled.checked=c.enabled;row.enabled.setAttribute('aria-label','Разрешить пробуждение: '+c.label);row.el.querySelector('.device-mac').textContent=c.mac;row.el.querySelector('.device-signal').textContent=c.ageSeconds!=null?c.rssi+' dBm':'';}
}
function renderEvents(events){
  const host=$('events');host.replaceChildren();if(!events.length){const empty=document.createElement('div');empty.className='empty';empty.textContent='Пока нет событий';host.append(empty);return;}
  for(const event of events){const el=document.createElement('div');el.className='event';const time=document.createElement('time');time.textContent=age(event.ageSeconds);const message=document.createElement('p');message.textContent=event.message;el.append(time,message);host.append(el);}
}
async function refreshScan(){
  if(model.scanBusy||$('scanArea').hidden)return;model.scanBusy=true;try{const data=await api('scan');model.scans=data.devices||[];renderScan();}catch(error){feedback(error.message,true);}finally{model.scanBusy=false;}
}
function renderScan(){
  const host=$('scanList');if(!model.scanEmpty){host.replaceChildren();model.scanEmpty=document.createElement('div');model.scanEmpty.className='empty';host.append(model.scanEmpty);}
  const devices=model.scans.filter(d=>$('showAll').checked||d.xboxLikely||d.microsoft||d.registered).sort((a,b)=>b.rssi-a.rssi);
  model.scanEmpty.hidden=devices.length>0;model.scanEmpty.textContent=$('showAll').checked?'Устройств пока не видно. Включите контроллер рядом с ESP32.':'Подходящих устройств пока не видно. Включите геймпад или покажите все BLE-устройства.';
  const keys=new Set(devices.map(d=>d.mac+':'+d.addressType));for(const [key,row] of model.scanRows){if(!keys.has(key)){row.el.remove();model.scanRows.delete(key);}}
  for(const d of devices){const key=d.mac+':'+d.addressType;let row=model.scanRows.get(key);if(!row){const el=$('scanTemplate').content.firstElementChild.cloneNode(true);row={el,data:d};row.add=el.querySelector('.scan-add');row.add.addEventListener('click',()=>{const item=row.data;$('newMac').value=item.mac;$('newLabel').value=(item.name||'Xbox контроллер').slice(0,40);$('newAddressType').value=['public','random'].includes(item.addressType)?item.addressType:'any';$('newLabel').focus();$('newLabel').scrollIntoView({behavior:'smooth',block:'center'});feedback('MAC перенесён в форму. Проверьте название и нажмите «Добавить».');});model.scanRows.set(key,row);host.append(el);}row.data=d;row.el.querySelector('.scan-name').textContent=d.name||'Без имени';const registered=d.registered||model.rows.has(d.mac);const tag=row.el.querySelector('.tag');tag.textContent=registered?'Уже добавлен':d.xboxLikely?'Вероятно Xbox':d.microsoft?'Признак Microsoft':'BLE-устройство';tag.classList.toggle('neutral',!d.xboxLikely&&!d.microsoft&&!registered);row.el.querySelector('.scan-detail').textContent=d.mac+' · '+d.rssi+' dBm · '+age(d.ageSeconds)+' · '+d.addressType;row.add.disabled=registered;row.add.textContent=registered?'Добавлен':'Добавить';}
}
function validateMac(input,label){const value=normalizeMac(input.value);if(!macPattern.test(value)||value==='00:00:00:00:00:00'||value==='FF:FF:FF:FF:FF:FF'){feedback(label+': нужен MAC вида AA:BB:CC:DD:EE:FF.',true);input.focus();return null;}input.value=value;return value;}
function validateIp(input,label){if(!ipv4(input.value)){feedback(label+': нужен IPv4-адрес, например 192.168.1.100.',true);input.focus();return false;}input.value=input.value.trim();return true;}
$('masterEnabled').addEventListener('change',async()=>{const wanted=$('masterEnabled').checked;const ok=await write('master','master',{enabled:wanted},wanted?'Пробуждение включено.':'Пробуждение от всех контроллеров выключено.');if(!ok&&model.state)$('masterEnabled').checked=model.state.enabled;});
$('controllerForm').addEventListener('submit',async event=>{event.preventDefault();const mac=validateMac($('newMac'),'Bluetooth MAC');const label=$('newLabel').value.trim();if(!mac)return;if(!label){feedback('Введите название контроллера.',true);$('newLabel').focus();return;}if(model.rows.has(mac)){feedback('Этот контроллер уже есть в списке. Измените его название или переключатель в карточке.',true);return;}const ok=await write('add','controller',{mac,label,enabled:true,addressType:$('newAddressType').value},'Контроллер добавлен и активирован.');if(ok){$('controllerForm').reset();if(!$('scanArea').hidden)refreshScan();}});
$('scanButton').addEventListener('click',()=>{$('scanArea').hidden=false;refreshScan();});$('closeScan').addEventListener('click',()=>{$('scanArea').hidden=true;});$('showAll').addEventListener('change',renderScan);
function updateTargetFields(){
  const mode=$('targetMode').value;
  $('targetPort').disabled=mode==='ethernet';$('targetBroadcast').disabled=mode==='ethernet';
}
$('targetMode').addEventListener('change',updateTargetFields);
$('targetForm').addEventListener('input',()=>{model.targetDirty=true;updateBusy();});
$('targetForm').addEventListener('change',()=>{model.targetDirty=true;updateBusy();});
async function resolveTargetMac(){
  if(!validateIp($('targetIp'),'IP компьютера'))return false;
  if(model.pending.has('resolve'))return false;
  const requestedIp=$('targetIp').value;
  model.pending.add('resolve');updateBusy();$('resolveHint').textContent='Запрашиваем MAC через ARP… Компьютер должен быть включён.';
  try{
    const started=await api('resolve',{ip:requestedIp});
    const deadline=Date.now()+8000;
    while(Date.now()<deadline){
      await new Promise(resolve=>setTimeout(resolve,250));
      const result=await api('resolve');
      if(result.id!==started.id)throw new Error('Поиск изменён в другой вкладке. Повторите запрос.');
      if(result.state==='error')throw new Error(result.error||'MAC не найден');
      if(result.state==='success'){
        if($('targetIp').value!==requestedIp)throw new Error('IP изменился. Повторите поиск.');
        $('targetMac').value=result.mac;model.targetDirty=true;
        $('resolveHint').textContent=requestedIp+' → '+result.mac+'. MAC найден по ARP. Сохраните настройки компьютера.';
        feedback('MAC найден: '+result.mac);return true;
      }
    }
    throw new Error('ESP32 не завершила поиск. Проверьте соединение и повторите.');
  }catch(error){$('resolveHint').textContent=error.message;feedback(error.message,true);return false;}
  finally{model.pending.delete('resolve');if(model.state)renderState(model.state);updateBusy();}
}
$('resolveMac').addEventListener('click',()=>resolveTargetMac());
$('targetIp').addEventListener('input',()=>{$('targetMac').value='';$('resolveHint').textContent='IP изменён: прежний MAC сброшен. При сохранении найдём MAC нового компьютера.';});
$('targetForm').addEventListener('submit',async event=>{event.preventDefault();if(!validateIp($('targetIp'),'IP компьютера'))return;if(!$('targetMac').value.trim()&&!await resolveTargetMac())return;const mac=validateMac($('targetMac'),'MAC сетевой карты');if(!mac)return;if(parseInt(mac.slice(0,2),16)&1){feedback('MAC сетевой карты должен быть индивидуальным адресом, а не групповым.',true);$('targetMac').focus();return;}const broadcast=$('targetBroadcast').value.trim().toLowerCase()||'auto';if(broadcast!=='auto'&&!ipv4(broadcast)){feedback('Broadcast-адрес: введите auto или корректный IPv4-адрес.',true);$('targetBroadcast').focus();return;}const payload={ip:$('targetIp').value,mac,mode:$('targetMode').value,broadcast,port:Number($('targetPort').value),quietSeconds:Number($('quietSeconds').value),cooldownSeconds:Number($('cooldownSeconds').value)};const ok=await write('target','target',payload,'Настройки компьютера сохранены.');if(ok){model.targetDirty=false;$('targetBroadcast').value=broadcast;$('resolveHint').textContent='Сохранено: '+payload.ip+' → '+payload.mac+'. Для последующих пробуждений повторный поиск MAC не нужен.';if(model.state)renderState(model.state);}});
$('wifiForm').addEventListener('submit',async event=>{event.preventDefault();const ssid=$('wifiSsid').value;const password=$('wifiPassword').value;if(!ssid.trim()){feedback('Введите имя Wi-Fi сети.',true);$('wifiSsid').focus();return;}if(new TextEncoder().encode(ssid).length>32){feedback('SSID не должен превышать 32 байта. Для кириллицы это может быть меньше 32 символов.',true);return;}if(password.length>0&&(new TextEncoder().encode(password).length<8||new TextEncoder().encode(password).length>63)){feedback('Пароль WPA должен содержать от 8 до 63 байт. Пустой пароль подходит только для открытой сети.',true);$('wifiPassword').focus();return;}const ok=await write('wifi','wifi',{ssid,password},'Настройки Wi-Fi сохранены. ESP32 подключается к сети; при смене адреса откройте панель по новому IP.');if(ok){$('wifiPassword').value='';}});
refresh();setInterval(()=>{if(!document.hidden){refresh();refreshScan();}},3000);document.addEventListener('visibilitychange',()=>{if(!document.hidden){refresh();refreshScan();}});
</script>
</body>
</html>)HTML";
