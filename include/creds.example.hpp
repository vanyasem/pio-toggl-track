#ifndef CREDS_HPP
#define CREDS_HPP

#ifndef STASSID
#define STASSID "examplessid"
#define STAPSK "examplepassword"
#endif

#ifndef TOGGLUSERNAME
#define TOGGLUSERNAME "exampleusername"
#define TOGGLPASSWORD "examplepassword"
#endif

inline constexpr int creds_toggl_workspace_id = 1337;
inline constexpr int creds_toggl_user_id = 111;
inline constexpr int creds_toggl_project_1_id = 123;
inline constexpr int creds_toggl_project_2_id = 124;
inline constexpr float creds_toggl_payment_rate = 100.0;

#endif
