import re
import matplotlib.pyplot as plt

def leer_resultados(nombre_archivo):
    data = {
        60:   {"archivo": [], "costo": [], "violaciones": [], "t_ini": [], "t_iter": []},
        500:  {"archivo": [], "costo": [], "violaciones": [], "t_ini": [], "t_iter": []},
        1000: {"archivo": [], "costo": [], "violaciones": [], "t_ini": [], "t_iter": []},
        "metodo": nombre_archivo.replace(".txt", "")
    }

    with open(nombre_archivo, "r") as f:
        contenido = f.read()

    bloques = [b for b in contenido.split("-------------------------------------") if "Archivo:" in b]

    # Cada 8 bloques cambia la cantidad de iteraciones
    cantidades = [60, 500, 1000]
    
    for idx, b in enumerate(bloques):
        iteraciones = cantidades[idx // 8]

        archivo = int(re.search(r"Archivo:\s+(\d+)", b).group(1))
        t_ini   = float(re.search(r"Tiempo Sol Inicial\(ms\):\s+([0-9.]+)", b).group(1))
        t_iter  = float(re.search(r"Tiempo Iteraciones\(ms\):\s+([0-9.]+)", b).group(1))
        viol, costo = map(int, re.search(r"Mejor Sol:\s+(\d+)\s+(\d+)", b).groups())

        d = data[iteraciones]
        d["archivo"].append(archivo)
        d["costo"].append(costo)
        d["violaciones"].append(viol)
        d["t_ini"].append(t_ini)
        d["t_iter"].append(t_iter)

    return data


# Leer todos los métodos
archivos = ["Greedy.txt", "Naive.txt", "Aleatorio.txt"]
resultados = [leer_resultados(a) for a in archivos]


# -----------------------------
# GRAFICOS por iteraciones
# -----------------------------

def graficar_por_iter(iteracion_objetivo, campo, ylabel, titulo):
    plt.figure(figsize=(12,6))

    for r in resultados:
        metodo = r["metodo"]
        d = r[iteracion_objetivo]
        plt.plot(d["archivo"], d[campo], label=f"{metodo}")

    plt.title(f"{titulo} (Iteraciones = {iteracion_objetivo})")
    plt.xlabel("Archivo")
    plt.ylabel(ylabel)
    plt.grid(True)
    plt.legend()
    plt.show()


# COSTO para 60, 500, 1000
graficar_por_iter(60, "costo", "Costo Final", "Costo Final por Método")
graficar_por_iter(500, "costo", "Costo Final", "Costo Final por Método")
graficar_por_iter(1000, "costo", "Costo Final", "Costo Final por Método")

# VIOLACIONES
graficar_por_iter(60, "violaciones", "Violaciones", "Violaciones por Método")
graficar_por_iter(500, "violaciones", "Violaciones", "Violaciones por Método")
graficar_por_iter(1000, "violaciones", "Violaciones", "Violaciones por Método")

# TIEMPO ITERACIONES
graficar_por_iter(60, "t_iter", "Tiempo Iteraciones (ms)", "Tiempo de Iteración por Método")
graficar_por_iter(500, "t_iter", "Tiempo Iteraciones (ms)", "Tiempo de Iteración por Método")
graficar_por_iter(1000, "t_iter", "Tiempo Iteraciones (ms)", "Tiempo de Iteración por Método")
