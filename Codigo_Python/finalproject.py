# -*- coding: utf-8 -*-


####################
# HELPER FUNCTIONS #
####################


def load_bank_data(filename):
    """Read bank data from a file.

    Parameters
    ----------
    filename : str
        Path of the file with bank data.

    Returns
    -------
    mapping : dict
        Dictionary that maps bank codes (keys) to bank names (values).

    Examples
    --------
    >>> banks = load_bank_data('banks.txt')
    >>> banks['2080']
    'Abanca Corporacion Bancaria, S.A.'
    >>> banks['9000']
    'Banco de España'
    >>> len(banks)
    242
    """
    infile = open(filename,'r')
    diccionario = dict()
    lista = []
    with open(filename,'r', encoding='utf-8') as infile:
        for line in infile:
            lista = line.split('\t')
            lista[1] = lista[1].replace('\n','')
            diccionario[lista[0]] = lista[1]
    return diccionario

            
 
def replace_letters(alphanumeric):
    """Replace letters by digits in a string.
    
    Convert a sequence of alphanumeric characters (digits and letters)
    into a sequence of digits, in accordance with the following rule:
    
    - Digits '0' through '9' are not transformed
    - 'A' is converted into '10'
    - 'B' is converted into '11'
    - 'C' is converted into '12'
    ...
    - 'Y' is converted into '34'
    - 'Z' is converted into '35'

    Parameters
    ----------
    alphanumeric : str
        String of alphanumeric characters (digits and letters).

    Returns
    -------
    str
        String of digits.

    Examples
    --------
    >>> replace_letters('123')
    '123'
    >>> replace_letters('ES')
    '1428'
    >>> replace_letters('UK')
    '3020'
    >>> replace_letters('7BE0')
    '711140'
    """
    dictionary = {'A' : 10, 'G' : 16, 'M' : 22, 'S' : 28, 'Y' : 34,
    'B' : 11, 'H' : 17, 'N' : 23, 'T' : 29, 'Z' : 35, 'C' : 12, 'I' : 18, 
    'O' : 24, 'U' : 30, 'D' : 13, 'J' : 19, 'P' : 25, 'V' : 31, 'E' : 14, 
    'K' : 20, 'Q' : 26, 'W' : 32, 'F' : 15, 'L' : 21, 'R' : 27, 'X' : 33}
    new_alpha = ''
    for letter in alphanumeric:
        if(dictionary.get(letter) is None):
            new_alpha = new_alpha + letter
        else:
            new_alpha = new_alpha + str(dictionary.get(letter))
    return new_alpha


          
###########
# CLASSES #
###########


class BankAccount(object):
    """Model for a bank account.
    
    Examples
    --------
    >>> johny = BankAccount('John Doe', '539007547034', 'BE')
    >>> johny.owner
    'John Doe'
    >>> johny.bban
    '539007547034'
    >>> johny.country
    'BE'
    >>> johny.check_digits()
    '68'
    >>> johny.iban()
    'BE68539007547034'
    """ 
    def __init__(self, owner, bban, country):
        """"Crear atributos con los mismos nombres que los parámetros.
        
        Parameters
        ----------
        owner : str
            Name of the account owner.
        bban : str
            Sequence of digits and letters that represents
            the country-specific Basic Bank Account Number.
        country : str
            2-letter ISO country code.
        """
        self.owner = owner
        self.bban = bban
        self.country = country


    
    def check_digits(self):
        """Compute the IBAN check digits.

        Returns
        -------
        str
            2-digit string with the IBAN
        """
        toret = self.bban + replace_letters(self.country) + '00'
        toret = 98 - ((int(toret)) % 97)
        if toret < 10:
            toret = '0' + str(toret)
        else:
            toret = str(toret)
        return toret



    def iban(self):
        """Return the IBAN."""
        return self.country + self.check_digits() + self.bban



class SpanishBankAccount(BankAccount):
    """Modelo de cuenta bancaria española.
    
    Examples
    --------
    >>> pepe = SpanishBankAccount('José García', '14651234461234567890')
    >>> pepe.owner
    'José García'
    >>> pepe.bban
    '14651234461234567890'
    >>> pepe.country
    'ES'
    >>> pepe.check_digits()
    '08'
    >>> pepe.iban()
    'ES0814651234461234567890'
    >>> pepe.bank_code()
    '1465'
    >>> pepe.bank_name()
    'Ing Bank, N.V. Sucursal en España'
    >>> pepe.branch()
    '1234'
    >>> pepe.account()
    '1234567890'
    >>> isinstance(pepe, BankAccount)
    True
    """
    def __init__(self, owner, bban):
        """Run __init__ method of parent class with country='ES'."""
        super()
        self.owner = owner
        self.bban = bban
        self.country = 'ES'
        

    def bank_code(self):
        """Return the bank code."""
        return self.bban[:4]

    
    def bank_name(self):
        """Return the name of the bank."""
        diccionario = load_bank_data('banks.txt')
        return diccionario[self.bank_code()]


    def branch(self):
        """Return the code of a branch bank."""
        return self.bban[4:8]


    def account(self):
        """Return the account number."""
        return self.bban[10:]


    def valid_bban(self):
        """Determine wether a Spanish BBAN (former CCC) is correct."""
        check = self.bban[8:10]
        a = int(self.bank_code()[0])*4 + int(self.bank_code()[1])*8 + int(self.bank_code()[2])*5 + int(self.bank_code()[3])*10
        b = int(self.branch()[0])*9 + int(self.branch()[1])*7 + int(self.branch()[2])*3 + int(self.branch()[3])*6
        c = (a + b) % 11
        c = 11 - c
        if c == 11:
            c = 0
        elif c == 10:
            c = 1
        lista1 = [1, 2, 4, 8, 5, 10, 9, 7, 3, 6]
        lista2 = []
        d = 0
        for number in self.account():
            lista2.append(number)
        for acc, lis in zip(lista2, lista1):
            d += int(acc) * int(lis)
        e = d % 11
        e = 11 - e     
        confirm = str(c) + str(e)
        return check == confirm


    
if __name__ == '__main__':

    import doctest
    doctest.testmod(verbose=False)#, optionflags=doctest.FAIL_FAST)
    
    try:
        kiko = SpanishBankAccount('Francisco Pérez', '00302053091234567895')
        ana = SpanishBankAccount('Ana García', '12345678901234567890')
        assert kiko.valid_bban() is True
        assert ana.valid_bban() is False
        print('Congratulations! You got a bonus!')
    except:
        print('No bonus for you :(')