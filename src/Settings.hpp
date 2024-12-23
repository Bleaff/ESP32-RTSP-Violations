# ifndef Settings_hpp
# define Settings_hpp

class Settings
{
public:
	const char *ssid;
	const char *password;
  	int is_set = 0;
	Settings(const Settings &settings);
	Settings(const char *ssid, const char *password);
	void set_ssid(const char *ssid);
	void set_password(const char *password);
	Settings();
};

# endif