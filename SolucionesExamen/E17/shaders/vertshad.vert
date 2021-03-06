#version 330 core

in vec3 vertex;
in vec3 normal;

in vec3 matamb;
in vec3 matdiff;
in vec3 matspec;
in float matshin;



uniform mat4 proj;
uniform mat4 view;
uniform mat4 TG;
// ***** PARA PINTAR LA VACA *****
uniform float isPrintingVaca;

// ***** PARA CAMBIAR EL FOCO *****
uniform float changeFocus;

// ***** PARA EL SEGUNDO FOCO *****
uniform vec3 posFocus2;

// Valors per als components que necessitem dels focus de llum
vec3 colFocus; // ***** APROVECHAMOS LA VARIABLE PARA HACER EL CAMBIO DE FOCO *****
vec3 llumAmbient = vec3(0.2, 0.2, 0.2);
vec3 posFocus = vec3(0, 0, 0); // ***** 000 EQUIVALE AL CENTRO DE COORDENADAS EN SCO QUE ES DONDE SE SITUA EL OBSERVADOR

bool ambienteCalculado = false;

out vec3 fcolor;

vec3 Lambert (vec3 NormSCO, vec3 L) 
{
    // S'assumeix que els vectors que es reben com a paràmetres estan normalitzats

    // Inicialitzem color a component ambient
    // ***** EVITAMOS RECALCULAR LA AMBIENTE 2 VECES *****
     vec3 colRes = vec3(0.0,0.0,0.0);
    if(ambienteCalculado = false){
        colRes = llumAmbient * matamb;
        ambienteCalculado = true;
    }

    // Afegim component difusa, si n'hi ha
    if (dot (L, NormSCO) > 0){
    
        if(isPrintingVaca == 0){
            colRes = colRes + colFocus * matdiff * dot (L, NormSCO);
        }
        else { // ***** SI PINTAMOS UNA VACA INDICAMOS QUE LA COMPONENTE DIFUSA ES GRIS *****
            colRes = colRes + colFocus * vec3(0.75,0.75,0.75) * dot(L,NormSCO);
        }
    }
    return (colRes);
}

vec3 Phong (vec3 NormSCO, vec3 L, vec4 vertSCO) 
{
    // Els vectors estan normalitzats

    // Inicialitzem color a Lambert
    vec3 colRes = Lambert (NormSCO, L);

    // Calculem R i V
    if (dot(NormSCO,L) < 0)
      return colRes;  // no hi ha component especular

    vec3 R = reflect(-L, NormSCO); // equival a: normalize (2.0*dot(NormSCO,L)*NormSCO - L);
    vec3 V = normalize(-vertSCO.xyz);

    if ((dot(R, V) < 0) || (matshin == 0))
      return colRes;  // no hi ha component especular
    
    // Afegim la component especular
    float shine = pow(max(0.0, dot(R, V)), matshin);
    return (colRes + matspec * colFocus * shine); 
}

void main()
{	
    vec4 posFocusSCO;
    if(changeFocus == 0.0){
        colFocus = vec3(0.0, 0.8, 0.8);
        posFocusSCO = vec4(posFocus,1.0); // ***** NO MULTIPLICAMOS LA POSICIÓN DEL FOCO POR LA VIEW YA QUE QUEREMOS QUE SEA DE CAMARA*****
    }
    else{
        colFocus = vec3(0.8, 0.8, 0.8);
        posFocusSCO = view*vec4(vec3(1.0,1.0,1.0),1.0); // ***** MULTIMPLICAMOS POR LA VIEW PARA QUE SEA DE ESCENA *****
    }
        
    vec3 NormSCO = inverse(transpose(mat3(view*TG)))*normal; // ***** CALCULAMOS LA NORMAL EN SISTEMA DE COORDENADAS DE OBSERVADOR *****
    NormSCO = normalize(NormSCO); // ***** NORMALIZAMOS LA NORMAL *****
    vec4 v = view*TG*vec4(vertex,1.0);
    
    vec4 L = normalize(posFocusSCO - v); // ***** CALCULAMOS EL VECTOR L (VECTOR DIRECTOR ENTRE EL VERTICE Y LA POSICION DEL FOCO
    fcolor = Phong(NormSCO, L.xyz, v); // ***** CALCULAMOS PHONG Y SE LO PASAMOS AL SHADER
    
    // ***** PARA EL SEGUNDO FOCO *****
    colFocus = vec3(0.4,0.4,0.4);
    vec4 posFocus2SCO = view*vec4(posFocus2, 1.0);
    vec4 L2 = normalize(posFocus2SCO - v);
    fcolor = fcolor + Phong(NormSCO, L2.xyz, v);
    
    gl_Position = proj * view * TG * vec4 (vertex, 1.0);
}
