import re
import matplotlib.pyplot as plt
import os
import numpy as np

def leer_resultados(nombre_archivo):
    data_por_iteracion = {} 
    metodo = nombre_archivo.replace(".txt", "")
    
    if not os.path.exists(nombre_archivo):
        print(f"Advertencia: No se encontró {nombre_archivo}")
        return None

    with open(nombre_archivo, "r") as f:
        contenido = f.read()

    bloques_texto = contenido.split("-------------------------------------")

    for bloque in bloques_texto:
        if "Archivo:" not in bloque: continue

        try:
            archivo = int(re.search(r"Archivo:\s+(\d+)", bloque).group(1))
            iteraciones = int(re.search(r"Iteraciones:\s+(\d+)", bloque).group(1))
            
            # Tiempos
            t_ini = float(re.search(r"Tiempo Sol Inicial\(ms\):\s+([0-9.]+)", bloque).group(1))
            t_iter = float(re.search(r"Tiempo Iteraciones\(ms\):\s+([0-9.]+)", bloque).group(1))
            t_total = t_ini + t_iter
            
            # Mejor Sol: Bloques Penalizacion
            match_sol = re.search(r"Mejor Sol:\s+(\d+)\s+(\d+)", bloque)
            bloques = int(match_sol.group(1))
            penalizacion = int(match_sol.group(2))
            
            if iteraciones not in data_por_iteracion:
                data_por_iteracion[iteraciones] = {
                    "archivo": [], "bloques": [], "costo": [], 
                    "t_ini": [], "t_iter": [], "t_total": []
                }

            d = data_por_iteracion[iteraciones]
            d["archivo"].append(archivo)
            d["bloques"].append(bloques)
            d["costo"].append(penalizacion)
            d["t_ini"].append(t_ini)
            d["t_iter"].append(t_iter)
            d["t_total"].append(t_total)

        except AttributeError:
            continue

    return {"metodo": metodo, "datos": data_por_iteracion}

# Procesar archivos
archivos = ["Greedy.txt", "Naive.txt", "Aleatorio.txt"]
resultados = [res for f in archivos if (res := leer_resultados(f))]

# ---------------------------------------------------------
# FUNCIÓN DE GRAFICADO GENÉRICA
# ---------------------------------------------------------
def graficar_metrica(iter_target, metrica, y_label, titulo, filename):
    plt.figure(figsize=(10, 6))
    hay_datos = False
    
    for item in resultados:
        metodo = item["metodo"]
        d = item["datos"].get(iter_target)
        
        if d:
            # Ordenar por instancia para que la linea salga ordenada
            pares = sorted(zip(d["archivo"], d[metrica]))
            x = [p[0] for p in pares]
            y = [p[1] for p in pares]
            
            plt.plot(x, y, marker='o', label=metodo)
            hay_datos = True

    if hay_datos:
        plt.title(f"{titulo} ({iter_target} Iteraciones)")
        plt.xlabel("Instancia (ID Archivo)")
        plt.ylabel(y_label)
        plt.grid(True, linestyle='--', alpha=0.7)
        plt.legend()
        plt.savefig(filename)
        print(f"Gráfico guardado: {filename}")
    plt.close()

# ---------------------------------------------------------
# GENERACIÓN DE TODOS LOS GRÁFICOS
# ---------------------------------------------------------
niveles = [60, 500, 1000]

for it in niveles:
    # 1. Penalización (Calidad)
    graficar_metrica(it, "costo", "Penalización", "Calidad de Solución", f"costo_{it}.pdf")
    
    # 2. Bloques (Recursos)
    graficar_metrica(it, "bloques", "Cantidad de Bloques", "Uso de Recursos", f"bloques_{it}.pdf")
    
    # 3. Tiempo de Búsqueda (Iteraciones)
    graficar_metrica(it, "t_iter", "Tiempo Iteraciones (ms)", "Tiempo de Búsqueda", f"tiempo_iter_{it}.pdf")

# 4. Tiempo TOTAL (Solo para 1000 iteraciones como representativo)
graficar_metrica(1000, "t_total", "Tiempo Total (ms)", "Tiempo Total de Ejecución", "tiempo_total_1000.pdf")

# ---------------------------------------------------------
# IMPRIMIR TABLA DE ESTADÍSTICAS (Para tu informe)
# ---------------------------------------------------------
print("\n--- ESTADÍSTICAS DE TIEMPO (1000 Iteraciones) ---")
print(f"{'Método':<15} | {'T. Init (ms)':<15} | {'T. Total (ms)':<15} | {'% Init del Total'}")
print("-" * 65)
for item in resultados:
    d = item["datos"].get(1000)
    if d:
        avg_ini = np.mean(d["t_ini"])
        avg_tot = np.mean(d["t_total"])
        ratio = (avg_ini / avg_tot) * 100
        print(f"{item['metodo']:<15} | {avg_ini:<15.4f} | {avg_tot:<15.4f} | {ratio:.6f}%")