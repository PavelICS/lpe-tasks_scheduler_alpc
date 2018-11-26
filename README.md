# POC: ALPC TasksScheduler LPE for Windows 7 and other

Позволяет заменить содержимое любого файла, удовлетворяющих условиям:
  1) Администратор имеет права в него писать
  2) Файл не занят другими программами
    2.1) Если это exe, то процесс не запущен
    2.2) Если это dll, то она в данный момент никем не используется
 
# Применение

Загрузить на целевую машину екслойт ALPC-TaskSched-LPE.exe и нагрузку
Запуск:
  ALPC-TaskSched-LPE.exe target_file [payload_file]
payload_file по умолчанию payload.exe

# Примеры
Нас больше интересуют те файлы, которые находятся в автозапуске в планировщике задач, как сервисы, через реестр и прочие способы автозапуска.

Примерами целевых приложений могут быть
  - Задача "GoogleUpdateTaskMachine"
  - Задача "Системное обновление Браузера Яндекс"
  - Сервис "Microsoft .NET Framework NGEN"
  
  
Задача GoogleUpdate запускается от имени системы
  ![screenshot](https://user-images.githubusercontent.com/45364791/49029476-01fc2600-f1a5-11e8-94a4-c0c3a2bf14f9.png)
  
Администратор имеет право писать в файл, отвечающий за обновления
  ![screenshot](https://user-images.githubusercontent.com/45364791/49029477-01fc2600-f1a5-11e8-9e03-ce507ba38f78.png)
 
Запуск експлойта для подмены файла обновления на произвольный другой
  ![screenshot](https://user-images.githubusercontent.com/45364791/49029478-01fc2600-f1a5-11e8-895e-6cf8e03d8ecd.png)
 
 При следующей попытке обновления отрабатывает подменённый файл от имени system, который запускает калькулятор
  ![screenshot](https://user-images.githubusercontent.com/45364791/49029479-01fc2600-f1a5-11e8-87e4-f468918cad28.png)






