# R30H-3 — референтно време за STOP, E22 pacing и двойна радиозагуба

**22.11.2026. Статус: SOURCE-SIMULATION + OBSERVABILITY PREPARED; REAL RF = NOT_RUN.** Разработката използва същия клон и същия хардуер; **няма включване на радио, мотори или соленоиди**. R30H-3 НЕ удостоверява безопасност за вода.

## Какво е разработено и какво доказва

- [`simulate_r30h3_rf_timing.py`](../tools/simulate_r30h3_rf_timing.py) чете истинските константи от `Remote/source/boat_radio.cpp`, `Boat/source/boat_dual_radio_port.cpp`, `Boat/source/boat_radio_safety.h` и `Boat/source/e22_uart_link.h`. Изброява фазите на E22 3000ms pacing през 1ms стъпка, без да измисля реални закъснения или експериментални резултати.
- [`test_r30h3_software_exposure.py`](../tests/test_r30h3_software_exposure.py) проверява източниците, exact boundary, 6000ms freshness, 10s RTH, нулева глава на hypothetical 30ms/frame duty и явно `NOT_RUN`.
- [`Remote/source/r30h3_stop_trace.h`](../../Remote/source/r30h3_stop_trace.h) и instrumentation в `Remote/source/boat_radio.cpp` разделят `STOP_BUTTON_REQUEST`, `E22_PACING_DEFERRED`, `NRF_BUFFER_ADMITTED/REJECTED` и `E22_UART_BUFFER_ADMITTED/REJECTED_PENDING`. 16-slot ring, локални millis, без allocation/serial/HMAC data. Докато `REMOTE_R30H3_STOP_TRACE_ENABLED=0`, няма logging overhead в активния source. Read-only API `boatRadio_takeR30H3StopTrace()` връща false; бъдещ test harness може да чете локалните записи само след разрешено non-actuating bring-up.
- [`Remote/tests/r30h3_remote_stop_trace_test.cpp`](../../Remote/tests/r30h3_remote_stop_trace_test.cpp) проверява реда на заявка/буфер/отлагане, препълване и отсъствие на загуба на най-старите записи. Няма полева физическа валидация.

## Инженерна граница от кодовите таймери — НЕ от RF измерване

| Събитие | Симулационен извод при идеален 1ms scheduler |
|---|---|
| E22 STOP веднага след предходно успешно E22 TX | **3000ms само изчакване за разрешен следващ прозорец**, плюс неизвестен UART + RF + Boat software + current-zero |
| E22 STOP при точно 3000ms след последно TX | 0ms pacing-only, но RF предаването пак **не е доказано** |
| nRF единствен валиден канал, RF прекъснат след приемане | `age >250ms`, първа stale милисекунда: **251ms** |
| E22 единствен валиден канал, RF прекъснат след приемане | `age >6000ms`, първа stale милисекунда: **6001ms** |
| Двата канала прекъснати при t=0; последен nRF пакет t=0, E22 packet age 0–2999ms | `BOTH_STALE` при **3002–6001ms** след t=0 при идеална стъпка; реалната event loop добавя неизвестно време |
| Условен RTH | **не преди 10000ms от последния приет автентикиран Boat пакет**, само при активна допустима мисия, CRC-valid matching Home, свежа GNSS/heading/geofence, валиден локален стоп/акумулатор/feedback, releases OFF |
| STOP текущо „queued=true“ на Remote | **само локално поставяне в опашката**, не RF доставка, не Boat ACK и не физически STOP |

**Внимание:** диапазонът 3002–6001ms приема предходен валиден E22 в последния 3-секунден слот и липса на допълнителни пакети след RF cut. Това е сценарий от кодовите константи, не гаранция за граници на реалния хардуер. При други форми на прекъсване или scheduler jitter времето трябва да се измери. По време на backup-only автономна мисия `fresh` на E22 не е разрешение за manual motor/release controls.

**Две различни времена:** `last accepted RX + 10000ms` е RTH eligibility. `BOTH_STALE` зависи от 250/6000ms. Следователно 10s не се броят от самото RF cut и не гарантират RTH точно на 10.000s след прекъсването.

## Защо не ускоряваме резервния STOP автоматично преди RF signoff

В E22 профила `EST_AIRTIME_MS=25` е непроверена константа. Remote прави heartbeat през 3000ms (идеално **1200 на час**). Дори ако 25ms/frame беше точен: 1200 × 25 = **30000ms ефир на час**, остават 6000ms до **програмния** 36000ms/h 1% бюджет за STOP и други пакети. При **хипотетични** 30ms/frame само heartbeat-и изчерпват 36000ms/h, без свободен бюджет за авариен STOP. 25/30ms **не са измерени**; действителните честотна подлента, E22 suffix, duty/LBT и RF пакетна продължителност остават **OPEN**. Алтернативна политика (приоритетен STOP извън 3s heartbeat) се допуска само след законово и техническо оразмеряване. Изкуствено увеличаване на E22 transmission frequency сега може да наруши допустимата RF работа, без да докаже доставка.

Блокировките на Boat `BOAT_NRF_PIN_AUDIT_SIGNED=0`, `BOAT_E22_RF_AIRTIME_AUDIT_SIGNED=0` и Remote `REMOTE_NRF_PIN_AUDIT_SIGNED=0`, `REMOTE_E22_RF_AIRTIME_AUDIT_SIGNED=0` **не се променят**. Планираният общ 3S4P pack/INA231 BUS-only/двигател MDD20A/EKM014 solenoids остават непроменени; независимият physical safety/STOP/current-zero все още не е избран/приет.

## Симулация, запис на данни и одобрение

При реален RF-only тест се използват едновременно Remote локална STOP trace, Boat R30H-2 локална trace, **един общ логически анализатор** за Remote button edge / Boat event / RF cut trigger и, в последващ stage само след допълнителните физически gate-ове, отделно motor/solenoid zero-current. Два `millis()` часовника не могат да бъдат директно изваждани.

```bash
python3 Boat/tools/simulate_r30h3_rf_timing.py
python3 -m unittest Boat/tests/test_r30h3_software_exposure.py -v
# Само hypothetical what-if, никога измерен RF airtime:
python3 Boat/tools/simulate_r30h3_rf_timing.py --hypothetical-airtime-ms 30
```

[16 измервателни / правни / независими hardware acceptance проверки](BOAT_R30H_3_STOP_RF_ACCEPTANCE.csv) започват от `NOT_RUN`; **approved_limit_ms е празен**, защото максимално допустимият физически STOP latency не е определен от потребителя/инженерната приемателна процедура. Нито един изход на симулатора не се използва за PASS на този gate. R30H-1 22 board tests и R30H original 22 scenarios също остават `NOT_RUN`.

### Следващо действие — R30H-4

За действителен RF тест: покупка/инспекция на конкретните ревизии, схема на carrier-а, проверка на 5V VCC/3.3V UART, RF load/антени, legal EU868 channel/airtime, provisioned keys извън repository. **Първо логически-only RF тест със задвижване разкачено**, после инженерно одобрен физически STOP/current-zero тест с отделни независими защитни блокировки. Измерете 30 повторения за E22 STOP и total-loss с най-лошите фази. Не преминавайте към water trials или RTH с истински мотори преди подписване на отделните acceptance gates.

**Release markers:** Boat VER014 / Remote VER028 frozen. Sonar model deferred.
