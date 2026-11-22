# R30H — изпитване на реалните nRF24/E22 радиомодули, STOP и условно RTH

**Дата:** 2026-11-22. **Статус:** ПРОЦЕДУРА ПОДГОТВЕНА; ФИЗИЧЕСКИ ТЕСТОВЕ **NOT_RUN**. **Етап:** RF-only без задвижване → безопасни dummy-load проверки след отделно одобрение. **Boat VER014 / Remote VER028** не се променят.

**Източници:** [R30G приоритет на диагностиката](BOAT_R30G_RADIO_DIAGNOSTIC_PRIORITY.md), [R30A ограничения за сглобяване](BOAT_R30A_PROTOTYPE_ASSEMBLY_HANDOFF.md), [R79 battery/радио bench](BOAT_R79_LOW_BATTERY_BENCH_TEST.md), [R30A gates](BOAT_R30A_READINESS_GATES.csv), \`Boat/source/boat_dual_radio_port.cpp\`, \`Remote/source/boat_radio.cpp\` и \`Remote/source/dual_radio_manager.cpp\`.

## 0. Критична граница: какво е и не е изпълнимо сега

- **Без физически инсталирани и подписани модули няма резултати или реални времена.** Текущият \`Boat/source/firmware/Flytec_Boat.ino\` е **неактуиращ диагностичен target**: няма \`BoatPhysicalDriver\`, липсва реален STOP/coil-current reader, актуалните radio GPIO/PHY одобрения са 0. Затова нито STOP→изходи, нито действително RTH движение могат да се измерят с текущия готов firmware.
- **RF-only** изпитването разрешава само след инспекция на купените платки, проверка на захранващите напрежения/нива, легален RF план, валидни антени и временно **неактуиращо** телеметрично/диагностично приложение с реалната HMAC/session/frame верификация. \`VM\` на MDD20A и \`VH\` на EKM014 са физически **РАЗКАЧЕНИ**; мотори и всички соленоиди са разкачени. Не се променят commissioning флагове само заради RF тест.
- **Logic-only** измерванията могат да ползват instrumented simulator/HAL за намерение \`INTENT_STOP\`/\`INTENT_NAV_HOME\` и изрично се обозначават \`SIMULATED\`, а не „реален STOP“.
- **Dummy-load** измервания с ток и реални изходи са ЗАБРАНЕНИ, докато R30A gates (24 OPEN) и R23/R25/R27 проверки (общо 81 NOT_RUN), общата DC STOP/изолация, watchdog, current-zero feedback, ограничител на соленоиди, захранващи кабели, предпазители и физически HAL не са отделно подписани. Няма отделни моторна и соленоидна батерии: една 3S4P шина за бъдещите \`VM\` + \`VH\`. R30H не отменя тези блокировки.

## 1. Използвана конфигурация и очаквана текуща логика

| Параметър | Конфигурация от кода; не е RF измерване |
|---|---|
| Boat MCU | ESP32-S3-DevKitC-1-N32R16V; кандидат GPIO map, все още неподписан |
| Remote MCU | Waveshare ESP32-S3-Touch-LCD-4.3 Standard; GPIO са предварителни |
| Основна връзка | nRF24L01+ PA/LNA, SPI, канал 76, 250 kbps, CRC16, Auto-ACK, \`setRetries(5,15)\`, начален \`RF24_PA_LOW\`; 32-байтов frame |
| Резервна | EBYTE E22-900T22S UART, 5V VCC, **3.3V TTL логика след измерване**, EU868/съвместим PHY и антена — все още НЕпотвърдени |
| Сигурност | Споделен провизиониран root key, успешен handshake, валиден HMAC, session epoch, sequence/replay guard; непотвърден пакет не възстановява връзката |
| TX heartbeat | nRF приблизително на 100 ms; E22 приблизително на 3000 ms, ограничени от runtime |
| Freshness — Boat и Remote | primary: \`<=250ms\` от последен приет автентикиран пакет; backup: \`<=6000ms\` |
| Remote primary recovery | изисква минимум 1500 ms recovery holdoff + 8 успешни primary наблюдения |
| Загуба | \`BOAT_RADIO_WATCHDOG_LOST\` само когато **и двата** канала не са fresh; тогава STOP намерение и release OFF. Това НЕ е моментът на физическото прекъсване |
| Условно RTH | Не преди \`lastAnyAuthenticatedRxMs + 10000ms\`, само ако **и двата** stale и има активна подходяща мисия, CRC-valid matching Home, валидна локална GNSS/heading/geofence/battery/STOP/release safety обратна връзка |
| Възстановяване | преди RTH: задържана подходяща мисия може да продължи след нов валиден пакет и interlock проверки; след detached RTH старият outbound маршрут **НЕ** се възобновява; manual takeover изисква primary, неутрални стикове и изрична команда |
| STOP бутон | Remote прави опит и по двата канала; \`true\` от send НЕ означава RF доставка или физическо спиране; проверява се Boat safe-output и отделно ACK |
| Backup ограничения | E22 разрешава аварийно STOP и изрично Home за вече валидна мисия; **не** разрешава manual motors, release, mission start или clear-STOP. E22 TX pacing 3s може да забави резервен STOP — измерва се отделно |
| R30G допълнителна диагностика | subtype-8 се изпраща само като optional primary nRF traffic след управляващия трафик; не refresh-ва управляващата свежест, не служи за STOP ACK |

**Риск за безопасността за sign-off:** след едновременно физическо прекъсване на двата ефирни сигнала E22 може да остане \`fresh\` до **6 s след последния автентикиран E22 пакет**. Докато E22 е fresh, software watchdog не сигнализира total loss. Manual motor lease може да спре по-рано, но **автономната мисия не трябва да се приема за незабавно спряна**; измерете отделно поведение при последен E22 heartbeat преди RF cut. При неудовлетворителен максимум R30H се маркира FAIL/ENGINEERING_CHANGE, без вода и без мотори.

## 2. Пособия и безопасна постановка

Изискват се: проверени **две nRF24** и **две E22** платки с подходящи антени/товар и кабели; двата ESP32 контролера; безопасни **current-limited** захранвания на логическите rails; мултицет и осцилоскоп за напрежения/пулсации; логически анализатор с **общ времеви часовник** и поне 4 канала; възможност за контролирано прекъсване на двата RF пътя чрез RF shield box/атенюатори или еквивалентна безопасна, разрешена настройка. Не включвайте PA предавател без подходяща антена или одобрен 50Ω RF load; не замествайте екраниран тест с незаконно активно заглушаване.

**Предложени probe/event точки** (не се предвиждат нови MCU GPIO assignments без бордов pin audit): \`INPUT_STOP\` от безопасно наблюдаван бутон/софтуерен timestamp; \`TX_STOP_ATTEMPT\`; \`AUTH_STOP_RX\` след успешна HMAC/sequence верификация на Boat; \`STOP_INTENT\` от application; \`LOGICAL_OUTPUT_OFF\` от неактуиращ HAL; \`RF_BLOCKED\` от общ test trigger; \`LAST_AUTH_RX\` от Boat при последен валиден пакет; \`BOTH_STALE\`; \`RTH_ENTRY\`; \`AUTH_LINK_RESTORED\`; \`MISSION_RESUME\`. За по-късен dummy-load етап \`DUMMY_CURRENT_ZERO\` се взима от независим токов датчик/осцилоскоп (не от MCP23017 GPIO/ACK). **Проба на debug сигнал не трябва да заемa boot/USB/RF/safety GPIO или да разрешава задвижване.**

Измерване на E2E между Remote и Boat: **само с общ логически анализатор/времева база** или доказано синхронизирани часовници. Два несинхронизирани USB Serial timestamp-а не могат да се изваждат. Ако има само serial-логове, записвайте отделни локални интервали и маркирайте E2E=\`UNMEASURABLE\`. Използвайте еднаква версия на firmware, идентични ключове с правилно провизиране (**никога не качвайте ключове в GitHub**) и запишете Git commit, борд/модул ревизия, RF настройки, разстояние, напрежения, антени, температура, retry counts и analyzer sample rate.

## 3. Тестова последователност

### A. Подготовка, без актуатори

1. Инспектирайте конкретните доставени SKU, антенни конектори, E22 UART TXD/AUX при 5V VCC, нива на I2C/SPI и пиновете; сравнете с R23/R25 и sign-off sheet. При неизпълнен gate — **STOP**, само unpowered inspection.
2. Заредете отделен, еднозначно етикетиран **RF-only non-actuating** диагностичен build, който използва действителните radio frame/handshake/verification кодове. Запишете hash; не включвайте физически HAL и не преименувайте диагностиката на завършен Boat runtime.
3. Проверете всички outgoing commands/frames без моторни/соленоидни захранвания. Нулевите изходи са преднамерени и **не доказват STOP latency**. Провизирайте ключове извън repository. Направете успешен secure handshake; документирайте \`nrfReady\`, \`e22Ready\`, backup PHY и периодите на реално приети валидни пакети.
4. Направете RF-only тест с nRF самостоятелно, E22 самостоятелно и двата едновременно. Ако не е потвърдена E22 interoperability, не я маркирайте PASS и не интерпретирайте „двойна“ загуба като реален dual-radio test.

### B. STOP — измервания

5. За всяко рамо направете поне **30 независими натискания** на STOP с различни моменти спрямо heartbeat/поточен трафик: основен nRF, резервен E22 (при временно недостъпен nRF), и двата налични; добавете случай с nRF зает (R30G diagnostic backlog не може да буферира) и E22 най-неблагоприятна 3s фаза.
6. На обща времева база изчислете \`t_auth - t_input\` (транспорт), \`t_intent - t_auth\` (Boat обработка), \`t_off - t_input\` (E2E логически STOP), \`t_zero - t_input\` (само след одобрен dummy-load). Отбелязвайте frame drops/timeouts и дали ACK=ACCEPTED/EXECUTED е дошъл **отделно**, без да го приемате за физическо потвърждение.
7. При всяка валидна STOP команда проверете нулево propulsion/release intent, никакво повторно изстрелване на release, latch задържан до изрично разрешено clear-STOP. Проверете дублиран STOP, невалиден HMAC, стар epoch/replay и session reset. Невалидна команда **не** може да възстанови freshness или да разреши output.

### C. Загуба и възстановяване — измервания

8. Съберете синхронизирани timestamp-и \`LAST_AUTH_RX\`, \`RF_BLOCKED\`, \`BOTH_STALE\`, \`LOGICAL_OUTPUT_OFF\`. Тествайте: само nRF loss (E22 healthy), само E22 loss (nRF healthy), и **едновременна загуба на двата**. Фиксирайте фаза на последните heartbeat-и (особено E22 3000ms период) и повтаряйте поне 30 пъти.
9. Изчислете \`t_both_stale - t_rf_blocked\`, \`t_off - t_both_stale\`, \`t_off - t_rf_blocked\`. Изрично отбележете че freshness timeout (250/6000 ms) не е същото като RF detection time; за автономно движение трябва да бъде потвърден практичен безопасен максимум **преди** физическо приемане.
10. С предварително валидно, CRC-съвпадащо Boat NVS Home и изпълняваща се допустима мисия инжектирайте едновременен loss: преди 10 000 ms от **последния приет authenticated frame** не може да има \`RTH_ENTRY\`. При достигане на прага, ако всички локални условия са валидни, очаквайте \`RTH_ENTRY\` и **само Home navigation** с нулеви release output. Не стартирайте реални мотори.
11. Повторете с (поотделно): без активна мисия, подготвена но незапочната мисия, грешен/повреден Home CRC, stale GNSS, invalid heading, geofence fail, critical/invalid INA231 sample, external STOP, release busy без нов current-zero feedback. Очакване: **никакъв RTH и STOP остава**. Невалидно локално доказателство никога не се заобикаля заради 10s timeout.
12. Възстановете валидна E22 връзка **преди 10s** и отделно nRF преди 10s: не трябва да се стартира автоматичен RTH. Допустима задържана автономна мисия може да се възобнови единствено след валидна автентикация и проверки; стари manual stick/release пакети не се replay-ват. Резервният E22 не дава права за освобождаване.
13. След потвърден \`RTH_ENTRY\` възстановете E22 и после nRF: outbound **не** започва отново; само валиден, изричен primary/manual takeover с неутрални джойстици и допустим локален safety state може да смени режима. При critical battery/physical STOP остава STOP дори при възстановена връзка.
14. Инжектирайте дълга outage, retries, power-cycle единично и на двата радиомодула, burst loss и optional R30F diagnostic traffic. Измерете дали subtype-8 остава информационен, не refresh-ва control freshness, не подтиска STOP и не спира heartbeat.

### D. След одобрена независима физическа безопасност — бъдещ dummy-load етап

15. Създайте отделна одобрена физическа тестова версия на Boat runtime с реален input reader, общ DC-rated STOP, watchdog, independent release limiter/current-zero, и одобрен HAL. Започнете с logic probes; едва след това токово ограничени **изчислени и оразмерени** dummy loads, никога истински соленоиди/мотори за първа проверка.
16. Измерете \`DUMMY_CURRENT_ZERO\` при STOP и total loss; захранващи спадове, reset/brownout, фалшив RF recovery, coil release one-hot, тока на двата изходни домейна от **едната обща 3S шина**. Никаква RTH команда не може да захрани реални releases. Ако реален физически safety gate липсва: маркирайте D-phase **BLOCKED**, не го симулирайте като PASS.

## 4. Журнал, анализ и критерии

- [Тестова матрица](BOAT_R30H_RF_TEST_MATRIX.csv) — всеки сценарий има уникален ID и начално състояние \`NOT_RUN\`. Тя е списък за подписване, **не доказателство за проведени тестове**.
- [Празен CSV на събития](BOAT_R30H_RF_EVENTS_TEMPLATE.csv) — един ред на събитие, една физически обща времева база, \`time_us\` монотонно. Пишете **измерени** стойности, не желани или симулирани, а за SIMULATED — отделно обозначен fixture.
- [Автоматичен анализ](../tools/analyze_r30h_rf_events.py) — проверява event order, RTH не по-рано от 10s и задължителни фази; печата измерени времена. **Няма твърди STOP budget-и по подразбиране**, защото физически не са договорени/измерени; \`--stop-limit-ms\` и \`--loss-stop-limit-ms\` са експлицитни gates за лабораторна проверка. Никакво \`NOT_RUN\` не става \`PASS\` чрез празен CSV.
- За всеки сценарий: минимум **30 проби** за измервания на латентност (отбележете sample count, min, median, p95, max, missed STOP, неверни RTH, release activity) и поне един записан отрицателен вариант за всеки safety interlock. Запазете raw CSV, analyzer output, осцилограми, test video ако е полезно, firmware hash, actual power readings, снимки на board revisions. Не качвайте pairing keys.
- **Задължителен PASS**: валидна STOP никога не оставя разрешен активен логически изход; всички release intents OFF при total loss/RTH; никакъв RTH преди 10s; RTH при всички валидни локални условия и само допустима активна мисия; липса на RTH при всеки невалиден gate; никакъв manual/release replay след възстановяване; никако subtype-8 командно влияние; доказана отделна физическа DC safety преди current measurements.
- **Отворени критерии за sign-off**: предел за \`INPUT_STOP → OUTPUT_OFF\` през nRF и резервен E22, \`RF_BLOCKED → OUTPUT_OFF\` при автономна мисия, и допустимо \`RTH_DUE → RTH_ENTRY\` закъснение на действителния application loop. Избира се след първите безопасни измервания и инженерна оценка, но **преди** превозното средство да бъде допуснато до вода. Не приемайте 6s freshness window или 3s backup pacing автоматично за безопасни.
- **Абсолютен FAIL/BLOCK**: моторен/coil load без подписан R30A hardware gate; тест с несинхронизирани E2E часовници, представен като измерен; отсъстващ валиден authenticated timestamp; RTH, започнал преди 10s; release ON след STOP; resurrected outbound след RTH; нарушена safety lock; липса на своевременно спиране според договорения лимит.

## 5. Запис на подписването

За финален R30H протокол попълнете: дата, инженер/свидетел, RF setup и локална регулаторна съвместимост, Board/Remote commit SHAs, версии и снимки, действителни rail/measured IO voltages, runtime mode (\`RF_ONLY\`, \`SIMULATED\`, \`DUMMY_LOAD_APPROVED\`), sample count и време на провеждане, raw log checksum, отчет от analyzer, критични аномалии, STOP/RTH критерии и документ за физическа готовност. \`NOT_RUN\`, \`BLOCKED\`, \`FAIL\`, \`PASS\` са различни състояния. Никоя точка не може да премине в PASS без доказателство.

**Следващ етап:** R30H-1 — real radio procurement/identity, power-level and antenna audit; R30H-2 — RF-only authenticated transport timestamps; R30H-3 — integrated application logic simulation; R30H-4 — physically approved dummy-load latency; R30H-5 — safety review and acceptance. **Няма нова официална firmware версия или физическо одобрение с този документ.**
