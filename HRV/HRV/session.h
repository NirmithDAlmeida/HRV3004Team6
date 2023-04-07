#ifndef SESSION_H
#define SESSION_H

#include <QObject>
#include <QString>
#include <QTimer>
#include <QDateTime>

/* Purpose of class: Session object to hold information about a session
 *
 * Data Members:
 * -int time: A integer representation of how long a therapy lasts
 * - int id: a id representation of the session number
 * - QTimer* timer: A QTimer object to keep track of how long a therapy should last
 * - QString challengeLevel: A string representation of what challengeLevel a therapy is set at
 * - qint64 breathInt: A integer to show the breath interval for session
 *
 * Class functions:
 * -getters for data members
 * -setters for data members based of other inputs
 */

class session{
private:
    int id;
    QTimer* timer;//not needed
    QString challengeLevel;
    int breathInt;

public:
    session();
    ~session();
    QString getName();
    int getTime();
    int getBreathInt();
    void setBreathInt();
    void setChallengeLevel();
    QString getChallengeLevel();
    QTimer* getTimer();
};
#endif // SESSION_H
