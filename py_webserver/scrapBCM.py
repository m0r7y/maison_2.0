from urllib.request import urlopen
from bs4 import BeautifulSoup


soup = BeautifulSoup(urlopen(
    'http://www.banque-centrale.mg/index.php'), 'html.parser')

tables0 = soup.find('strong', {'style': 'font-size:11px;'})
tables1 = soup.find('table', {'width': '98%'})

date = tables0.text.strip()
devise = [table.text.strip() for table in tables1.find_all('strong')]

print('Devise du %s' % date)
print('1 EUR : AR %s PLUS BAS    AR %s PLUS HAUT' % (devise[3], devise[4]))
print('1 USD : AR %s PLUS BAS    AR %s PLUS HAUT' % (devise[6], devise[7]))
