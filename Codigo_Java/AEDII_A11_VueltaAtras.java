/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package aedii_a11_vueltaatras;

/**
 *
 * @author mdaraujo19_esei.uvig
 */
public class AEDII_A11_VueltaAtras {
    
    
    public static boolean colocarReinas (int reina, int[] tablero){
        int fila=0;
        boolean objetivo = false;
        while(fila<8 && !objetivo){
            tablero[reina] = fila;
            if(buenSitio(reina ,tablero)){ 
                if(reina==7)
                    objetivo = true;
                else{
                    objetivo = colocarReinas(reina+1,tablero);
                }   
            }
            fila++;
        }
        return objetivo;
    }
    
    private static boolean buenSitio (int r, int[] tabl)
    {
        // ¿Es amenaza colocar la reina “r” en A[r], con las anteriores?
        int i;
        for(i = 0; i < r; ++i)
        {
        if (tabl[i] == tabl[r]) 
            return false;
        if (Math.abs(i-r) == Math.abs(tabl[i]-tabl[r])) 
            return false;
        }
        return true;
    }
    
    public static boolean ensayar(char [][] laberinto, int posicionX, int posicionY){
        boolean objetivo = false;
        
        while(!objetivo){
            
            if (puedoMover(laberinto,posicionX,posicionY))
            //Teleport
                if(laberinto[posicionX][posicionY] == 'T'){
                        laberinto[posicionX][posicionY] = 'C';
                        int[] posicionT = buscarOtraT(laberinto, posicionX, posicionY);
                        posicionX= posicionT[0];
                        posicionY= posicionT[1];
                        laberinto[posicionX][posicionY] = 'C';
                }
                else
                    laberinto[posicionX][posicionY] = 'C';
            else
                return false;
            
            if(posicionX == laberinto.length-1 && posicionY == laberinto.length-1)
                objetivo = true;
            else{
                objetivo = ensayar(laberinto,posicionX+1,posicionY) || ensayar(laberinto,posicionX,posicionY+1) || 
                     ensayar(laberinto,posicionX-1,posicionY) || ensayar(laberinto,posicionX,posicionY-1);
                if(!objetivo){
                    laberinto[posicionX][posicionY] = 'I';
                }
            }
            
        }
        return objetivo;
    }
    
    private static boolean puedoMover (char [][] laberinto, int posicionX, int posicionY){
        if(posicionX < laberinto.length && posicionX > -1 && posicionY < laberinto.length && posicionY > -1 && 
                (laberinto[posicionX][posicionY]==' ' || laberinto[posicionX][posicionY]=='T') )
            return true;
        return false;
    }
    
    private static int[] buscarOtraT (char [][] laberinto, int posicionX, int posicionY){
        for(int i = 0 ; i < laberinto.length; i++)
            for(int j = 0 ; j < laberinto.length; j++)
                if(laberinto[i][j] == 'T')
                    return new int[] {i,j};
        return null;
    }
    
    public static boolean ensayarPalabras (char [][] laberinto, int posicionX, int posicionY, String cadena, int posCad){
        boolean objetivo = false;
        while (!objetivo){
            if(posicionValida(laberinto,posicionX,posicionY) && laberinto[posicionX][posicionY] == cadena.charAt(posCad)){
                    laberinto[posicionX][posicionY] = ' ';
            }
            else
                return false;
            if (posicionX == laberinto.length-1 && posicionY == laberinto.length-1)
                objetivo = true;
            else{
                objetivo = ensayarPalabras (laberinto,posicionX,posicionY+1,cadena,(posCad+1)%cadena.length())||
                        ensayarPalabras (laberinto,posicionX+1,posicionY,cadena,(posCad+1)%cadena.length())||
                        ensayarPalabras (laberinto,posicionX,posicionY-1,cadena,(posCad+1)%cadena.length())||
                        ensayarPalabras (laberinto,posicionX-1,posicionY,cadena,(posCad+1)%cadena.length());
                    
            }
        }
        return objetivo;
    }
    
    private static boolean posicionValida (char [][] laberinto, int posicionX, int posicionY){
        if(posicionX > -1 && posicionX < laberinto.length && posicionY > -1 && posicionY < laberinto.length)
            return true;
        else
            return false;
    }

    /**
     * @param args the command line arguments
     */
    public static void main(String[] args) {
        // TODO code application logic here
    }
    
}
