# Desktop приложение для библиотеки

## 🔗 Функционал авторизации/регистрации в библиотеку:
- Два уровня доступа в библиотеку:
  - Пользователь;
  - Сотрудник библиотеки.
- Хэширование паролей (QCryptographicHash);
- Валидация почты (QRegularExpressionValidator);

<img src="Res/reg_user.png" width="600" height="400">
Если допустить ошибку в почте, то она будет выделяться красным. 
<img src="Res/reg_user_mail.png" width="600" height="400">
<img src="Res/log_user.png" width="600" height="400">
<img src="Res/log_admin.png" width="600" height="400">

## 🔗 Основная часть приложения:

- Для пользователя:
  - Показать все книги;
  - Показать список любимых книг;
  - Показать спиос взятых книг;
  - Поиск книги по названию с функцией подсказок (QCompleter);
  - Фильтр поиска по жанру и/или имени автора;
  - Возможность "взять", "лайкнуть" книгу.

- Для администратора:
  - Показать все книги;
  - Показать список пользователей
  - Блокировка пользователей с функцией изъятия книги;
  - Показать спиос ВСЕХ взятых книг и кем они были взяты;
  - Поиск книги по названию с функцией подсказок (QCompleter);
  - Фильтр поиска по жанру и/или имени автора;

<img src="Res/main_user.png" width="600" height="400">


