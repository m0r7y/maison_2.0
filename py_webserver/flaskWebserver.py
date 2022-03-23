import time
import datetime
from functools import wraps

import flaskext.mysql
from celery import Celery
from flask import Flask, redirect, render_template, request, session, url_for
from pymysql.cursors import DictCursor

from pyarduino import *


def make_celery(app):
    celery = Celery(app.import_name, backend=app.config['CELERY_RESULT_BACKEND'],
                    broker=app.config['CELERY_BROKER_URL'])
    celery.conf.update(app.config)
    TaskBase = celery.Task

    class ContextTask(TaskBase):
        abstract = True

        def __call__(self, *args, **kwargs):
            with app.app_context():
                return TaskBase.__call__(self, *args, **kwargs)

    celery.Task = ContextTask
    return celery


flask_app = Flask(__name__)

"""flask_app.config.update(
    CELERY_BROKER_URL='redis://localhost:6379',
    CELERY_RESULT_BACKEND='redis://localhost:6379'
)


flask_app.config.update(
    CELERYBEAT_SCHEDULE={
        'log-every-20-seconds': {
            'task': 'test',
            'schedule': datetime.timedelta(seconds=30),
            'args': ('a', 'e', 30)
        },
    },
)

celery = make_celery(flask_app)


@celery.task(name="test")
def chambre1(cm0, cm1, temps):
    if not ser.is_open:
        ser.open()
    x = cm0.encode('utf-8')
    y = cm1.encode('utf-8')
    ser.write(x)
    time.sleep(int(temps/2))
    ser.write(y)
"""

mysql = flaskext.mysql.MySQL(cursorclass=DictCursor)
# config MariaDB
flask_app.config['MYSQL_DATABASE_USER'] = 'root'
flask_app.config['MYSQL_DATABASE_PASSWORD'] = 'tux87654321'
flask_app.config['MYSQL_DATABASE_DB'] = 'homeserver'
flask_app.config['MYSQL_DATABASE_HOST'] = 'localhost'

mysql.init_app(flask_app)


@flask_app.route('/')
@flask_app.route('/home')
def index():
    return render_template('home.html')


def is_logged_in(f):
    @wraps(f)
    def wrap(*args, **kwargs):
        if 'logged_in' in session:
            return f(*args, **kwargs)
        else:
            return redirect(url_for('login'))

    return wrap


@flask_app.route('/login', methods=['GET', 'POST'])
def login():
    if request.method == 'POST':
        given_login = request.form['login']
        given_password = request.form['password']

        cursor = mysql.get_db().cursor()
        result = cursor.execute('SELECT * from userinformation WHERE username = %s',
                                given_login)

        if result > 0:
            # work fine, dict type
            data = cursor.fetchone()
            if data['password'] == given_password:
                # flask_app.logger.info('(y)')
                session['logged_in'] = True
                session['username'] = given_login
                session['status'] = data['id']
                return redirect(url_for('cmd'))
            else:
                error = 'Invalid password'
        else:
            error = 'User name not found'
        cursor.close()
        return render_template('login.html', error=error)
    else:
        return render_template('login.html')


@flask_app.route('/commande', methods=['GET', 'POST'])
@is_logged_in
def cmd():
    #flask_app.logger.info(state_all)
    if request.method != 'POST':
        return render_template('commande.html',  data=get_state(ser, 11))
    cede = request.form['commande']
    if cede in ['0', '1', '2', '3', '4', '6', '7', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'i', 'j', 'k', 'l', 'm', 'o', 't', 'u', 'v', 'w', 'x', 'y', 'z']:
        cmd_ard(ser, cede)
    if cede in ['h', 'n']:
        cmd_ard(ser1, cede)
    return render_template('/commande.html', data=get_state(ser, 11))


@flask_app.route('/configuration', methods=['GET', 'POST'])
@is_logged_in
def configuration():
    # work fine
    global dataPort
    if request.method == 'POST':
        if session['status'] == 1:
            wrt = []
            try:
                file = open('configFile', 'r+')
                config = [el.rstrip() for el in file.readlines()]
                file.fseek(0, 1)
            except:
                file = open('configFile', 'w')
                config = ['/dev/ttyACM1', '9600', '/dev/ttyACM0', '9600']
            if request.form['device']:
                wrt.append(request.form['device'])
            else:
                wrt.append(config[0])
            if request.form['port']:
                wrt.append(request.form['port'])
            else:
                wrt.append(config[1])
            if request.form['device1']:
                wrt.append(request.form['device1'])
            else:
                wrt.append(config[2])
            if request.form['port1']:
                wrt.append(request.form['port1'])
            else:
                wrt.append(config[3])
            file.writelines("%s\n" % l for l in wrt)
            file.close()
            dataPort = wrt
            configure_serial(ser, wrt)
        return render_template('configuration.html', data=dataPort, ses=session['status'])
    return render_template('configuration.html', data=dataPort, ses=session['status'])


@flask_app.route('/comptes', methods=['GET', 'POST'])
@is_logged_in
def comptes():
    cursor = mysql.get_db().cursor()
    cursor.execute('SELECT * FROM userinformation')
    data = cursor.fetchall()
    cursor.close()
    if request.method == 'POST':
        if session['status'] == 1:
            if request.form['nom'] and request.form['password']:
                if request.form['password'] == request.form['vpassword']:
                    given_login = request.form['nom']
                    given_passe = request.form['password']
                    cursor = mysql.get_db().cursor()
                    result = cursor.execute('SELECT * from userinformation WHERE username = %s',
                                            given_login)
                    if result == 0:
                        cursor.execute('INSERT INTO userinformation(username, password, status) VALUES(%s, %s, %s)',
                                       (given_login, given_passe, 'guest'))
                        # attention with commit
                        mysql.get_db().commit()
                        message = 'successfully entered in DB'
                    else:
                        message = 'Username already in use'
                    cursor.close()
                else:
                    message = 'Password doesn\'t match'
            else:
                message = 'Fill all blanks'
        else:
            message = 'You are not root'
        return render_template('comptes.html', message=message, data=data, ses=session['status'])
    else:
        return render_template('comptes.html', data=data, ses=session['status'])


@flask_app.route('/logout')
def logout():
    session.clear()
    return redirect(url_for('login'))


@flask_app.route('/delete', methods=['GET', 'POST'])
@is_logged_in
def delete():
    if request.method == 'POST':
        if session['status'] == 1:
            accounts = request.form.getlist('delete')
            cursor = mysql.get_db().cursor()
            for account in accounts:
                cursor.execute('SELECT * from userinformation WHERE username = %s',
                               account)
                data = cursor.fetchone()
                if data['id'] != 1:
                    cursor.execute(
                        'DELETE FROM userinformation WHERE username = %s', account)
                    mysql.get_db().commit()
            cursor.close()
        return redirect(url_for('comptes'))
    else:
        return redirect(url_for('comptes'))


@flask_app.route('/notes', methods=['GET', 'POST'])
@is_logged_in
def notes():
    cursor = mysql.get_db().cursor()
    cursor.execute('SELECT * FROM notes ORDER BY date_p DESC LIMIT 10')
    data = cursor.fetchall()
    error = ''
    if request.method == 'POST':
        contenu = request.form['contenu']
        if contenu != '':
            cursor.execute(
                'INSERT INTO notes(op, contenu) VALUES(%s, %s)', (session['username'], contenu))
            mysql.get_db().commit()
            return redirect(url_for('notes'))
        else:
            error = 'Aucun contenu'
        cursor.close()
        return render_template('/notes.html', data=data, error=error)
    else:
        cursor.close()
        return render_template('/notes.html', data=data, error=error)


""""@flask_app.route('/tasks', methods=['GET', 'POST'])
@is_logged_in
def tasks():
    if request.method == 'POST':
        given_name = request.form['taskid']
        given_arg0 = request.form['arg0']
        given_arg1 = request.form['arg1']
        given_arg2 = int(request.form['arg2'])

        flask_app.config.update(
             CELERYBEAT_SCHEDULE={
                given_name: {
                'task': 'test',
                'schedule': datetime.timedelta(seconds=given_arg2),
                'args': (given_arg0, given_arg1)
                },
            },
        )
        return redirect(url_for('cmd'))
    else:
        return redirect(url_for('cmd'))
"""
if __name__ == '__main__':
    flask_app.secret_key = 'morty4321'
    flask_app.run(port=5000, debug=True)
