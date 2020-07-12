int eo_scram_spam(int scram[], int eo_list[][30], int fb, int rl, int ud,
                  int m, int b, int niss, int h);
int dr_scram_spam(int scram[], int dr_list[][30], int fb, int rl, int ud,
                  int m, int b, int h);
int drfrom_scram_spam(int scram[], int dr_list[][30], int from, int fb,
                      int rl, int ud, int m, int b, int niss, int hide);
int htr_scram_spam(int scram[], int htr_list[][30], int from,
                   int m, int b, int niss, int hide);
int dr_corners_scram_spam(int scram[], int sol[][30], int from, int m, int b,
                          int ignore);             
int dr_finish_scram_spam(int scram[], int sol[][30], int from, int m, int b);
int htr_finish_scram_spam(int scram[], int sol[][30], int m, int b);
int solve_scram(int scram[], int sol[][30], int m, int b, int optimal);
int reach_state(int eofb, int coud, int ep, int cp, int sol[][30]);
