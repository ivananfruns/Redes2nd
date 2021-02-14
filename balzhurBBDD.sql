-- phpMyAdmin SQL Dump
-- version 5.0.2
-- https://www.phpmyadmin.net/
--
-- Servidor: localhost
-- Tiempo de generación: 14-02-2021 a las 02:57:52
-- Versión del servidor: 8.0.19
-- Versión de PHP: 7.4.5RC1

SET SQL_MODE = "NO_AUTO_VALUE_ON_ZERO";
START TRANSACTION;
SET time_zone = "+00:00";


/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8mb4 */;

--
-- Base de datos: `balzhur`
--

-- --------------------------------------------------------

--
-- Estructura de tabla para la tabla `enemigos`
--

CREATE TABLE `enemigos` (
  `ID` int NOT NULL,
  `Name` varchar(64) COLLATE utf8_unicode_ci NOT NULL,
  `Race` tinyint NOT NULL,
  `Life` int NOT NULL,
  `Attack` int NOT NULL,
  `Defense` int NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

--
-- Volcado de datos para la tabla `enemigos`
--

INSERT INTO `enemigos` (`ID`, `Name`, `Race`, `Life`, `Attack`, `Defense`) VALUES
(1, 'Aleth', 1, 3, 7, 3),
(2, 'Yatza', 4, 6, 5, 3),
(3, 'Noth', 2, 4, 3, 7);

-- --------------------------------------------------------

--
-- Estructura de tabla para la tabla `inventarios`
--

CREATE TABLE `inventarios` (
  `ID` int NOT NULL,
  `Username_FK` varchar(64) CHARACTER SET utf8 COLLATE utf8_unicode_ci NOT NULL,
  `ObjectType` tinyint NOT NULL,
  `ObjectName` varchar(64) CHARACTER SET utf8 COLLATE utf8_unicode_ci NOT NULL,
  `Power` int NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

-- --------------------------------------------------------

--
-- Estructura de tabla para la tabla `objetos`
--

CREATE TABLE `objetos` (
  `ID` int NOT NULL,
  `ObjectType` tinyint NOT NULL,
  `Nombre` varchar(64) COLLATE utf8_unicode_ci NOT NULL,
  `Power` int NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

--
-- Volcado de datos para la tabla `objetos`
--

INSERT INTO `objetos` (`ID`, `ObjectType`, `Nombre`, `Power`) VALUES
(1, 0, 'Pocion de vida pequeña', 2),
(2, 0, 'Pocion de vida media', 4),
(3, 1, 'Espada pequeña', 3),
(4, 1, 'Daga de Hierro', 2),
(5, 2, 'Pechera de Tela', 3),
(6, 2, 'Pantalones de Cuero', 5);

-- --------------------------------------------------------

--
-- Estructura de tabla para la tabla `personajes`
--

CREATE TABLE `personajes` (
  `ID` int NOT NULL,
  `Username_FK` varchar(64) COLLATE utf8_unicode_ci NOT NULL,
  `Race` tinyint NOT NULL,
  `Name` varchar(64) COLLATE utf8_unicode_ci NOT NULL,
  `Life` int NOT NULL DEFAULT '3',
  `Attack` int NOT NULL DEFAULT '3',
  `Defense` int NOT NULL DEFAULT '3',
  `Magica` int NOT NULL DEFAULT '3'
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

-- --------------------------------------------------------

--
-- Estructura de tabla para la tabla `usermaptimes`
--

CREATE TABLE `usermaptimes` (
  `ID` int NOT NULL,
  `Username` varchar(64) COLLATE utf8_unicode_ci NOT NULL,
  `MapID` tinyint NOT NULL,
  `InitTime` timestamp NOT NULL,
  `EndTime` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

-- --------------------------------------------------------

--
-- Estructura de tabla para la tabla `users`
--

CREATE TABLE `users` (
  `ID` int NOT NULL,
  `Username` varchar(64) COLLATE utf8_unicode_ci NOT NULL,
  `Password` varchar(64) COLLATE utf8_unicode_ci NOT NULL,
  `NumEnemigosMatados` int NOT NULL DEFAULT '0',
  `NumMuertes` int NOT NULL DEFAULT '0'
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

-- --------------------------------------------------------

--
-- Estructura de tabla para la tabla `usersesiontimes`
--

CREATE TABLE `usersesiontimes` (
  `ID` int NOT NULL,
  `Username` varchar(64) COLLATE utf8_unicode_ci NOT NULL,
  `InicioSesion` timestamp NOT NULL,
  `FinalSesion` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

--
-- Índices para tablas volcadas
--

--
-- Indices de la tabla `enemigos`
--
ALTER TABLE `enemigos`
  ADD PRIMARY KEY (`ID`),
  ADD UNIQUE KEY `Name` (`Name`);

--
-- Indices de la tabla `inventarios`
--
ALTER TABLE `inventarios`
  ADD PRIMARY KEY (`ID`),
  ADD KEY `Username` (`Username_FK`);

--
-- Indices de la tabla `objetos`
--
ALTER TABLE `objetos`
  ADD PRIMARY KEY (`ID`);

--
-- Indices de la tabla `personajes`
--
ALTER TABLE `personajes`
  ADD PRIMARY KEY (`ID`),
  ADD KEY `FK_Personaje-User` (`Username_FK`);

--
-- Indices de la tabla `usermaptimes`
--
ALTER TABLE `usermaptimes`
  ADD PRIMARY KEY (`ID`),
  ADD KEY `Username` (`Username`);

--
-- Indices de la tabla `users`
--
ALTER TABLE `users`
  ADD PRIMARY KEY (`ID`),
  ADD UNIQUE KEY `Username` (`Username`);

--
-- Indices de la tabla `usersesiontimes`
--
ALTER TABLE `usersesiontimes`
  ADD PRIMARY KEY (`ID`),
  ADD KEY `Username` (`Username`);

--
-- AUTO_INCREMENT de las tablas volcadas
--

--
-- AUTO_INCREMENT de la tabla `enemigos`
--
ALTER TABLE `enemigos`
  MODIFY `ID` int NOT NULL AUTO_INCREMENT, AUTO_INCREMENT=4;

--
-- AUTO_INCREMENT de la tabla `inventarios`
--
ALTER TABLE `inventarios`
  MODIFY `ID` int NOT NULL AUTO_INCREMENT;

--
-- AUTO_INCREMENT de la tabla `objetos`
--
ALTER TABLE `objetos`
  MODIFY `ID` int NOT NULL AUTO_INCREMENT, AUTO_INCREMENT=7;

--
-- AUTO_INCREMENT de la tabla `personajes`
--
ALTER TABLE `personajes`
  MODIFY `ID` int NOT NULL AUTO_INCREMENT;

--
-- AUTO_INCREMENT de la tabla `usermaptimes`
--
ALTER TABLE `usermaptimes`
  MODIFY `ID` int NOT NULL AUTO_INCREMENT;

--
-- AUTO_INCREMENT de la tabla `users`
--
ALTER TABLE `users`
  MODIFY `ID` int NOT NULL AUTO_INCREMENT;

--
-- AUTO_INCREMENT de la tabla `usersesiontimes`
--
ALTER TABLE `usersesiontimes`
  MODIFY `ID` int NOT NULL AUTO_INCREMENT;

--
-- Restricciones para tablas volcadas
--

--
-- Filtros para la tabla `inventarios`
--
ALTER TABLE `inventarios`
  ADD CONSTRAINT `FK_Inventario-User` FOREIGN KEY (`Username_FK`) REFERENCES `users` (`Username`) ON DELETE CASCADE ON UPDATE CASCADE;

--
-- Filtros para la tabla `personajes`
--
ALTER TABLE `personajes`
  ADD CONSTRAINT `FK_Personaje-User` FOREIGN KEY (`Username_FK`) REFERENCES `users` (`Username`) ON DELETE CASCADE ON UPDATE CASCADE;

--
-- Filtros para la tabla `usermaptimes`
--
ALTER TABLE `usermaptimes`
  ADD CONSTRAINT `FK_usermaptimes-users` FOREIGN KEY (`Username`) REFERENCES `users` (`Username`) ON DELETE CASCADE ON UPDATE CASCADE;

--
-- Filtros para la tabla `usersesiontimes`
--
ALTER TABLE `usersesiontimes`
  ADD CONSTRAINT `FK_usersesiontimes-users` FOREIGN KEY (`Username`) REFERENCES `users` (`Username`) ON DELETE CASCADE ON UPDATE CASCADE;
COMMIT;

/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
